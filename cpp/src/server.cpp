#include <iostream>
#include <boost/shared_ptr.hpp>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/filesystem.hpp>
#include <boost/algorithm/string.hpp>
#include <vector>
#include <string>
#include "cmds.h"
#include "engine.h"
#include "config.h"
#include "packets.h"

using namespace std;
using namespace boost::asio::ip;

class ClientChannel;

vector<ClientChannel *> clientChannels;

class Listener
{
    boost::asio::io_service &ioService;
    tcp::acceptor acceptor;

public:
    Listener(boost::asio::io_service &ioService_)
        : ioService(ioService_), acceptor(ioService_, tcp::endpoint(tcp::v4(), 8473))
    {
    }
    void startAccept()
    {
        boost::shared_ptr<tcp::socket> socket(new tcp::socket(ioService));

        acceptor.async_accept(*socket, boost::bind(&Listener::handleAccept, this, socket, boost::asio::placeholders::error));
    }
    void handleAccept(boost::shared_ptr<tcp::socket> socket, const boost::system::error_code &error);
};

Game game;

void testHandler(const boost::system::error_code &error, size_t numSent)
{
    if (!error)
    {
        cout << "woo! " << numSent << endl;
    }
    else
    {
        cout << "boooooo" << endl;
    }
}

void clearVchAndPackResyncPacket(vch *dest)
{
    dest->clear();
    game.pack(dest);

    vch prepended;
    packToVch(&prepended, "C", PACKET_RESYNC_CHAR);
    packToVch(&prepended, "Q", (uint64_t)(dest->size()));

    dest->insert(dest->begin(), prepended.begin(), prepended.end());
}
void clearVchAndPackFrameCmdsPacket(vch *dest, FrameEventsPacket fcp)
{
    dest->clear();

    fcp.pack(dest);

    vch prepended;
    packToVch(&prepended, "C", PACKET_FRAMECMDS_CHAR);
    packToVch(&prepended, "Q", (uint64_t)dest->size());

    dest->insert(dest->begin(), prepended.begin(), prepended.end());
}

vector<boost::shared_ptr<Cmd>> pendingCmds;

class ClientChannel
{
    boost::asio::io_service &ioService;
    boost::shared_ptr<tcp::socket> socket;

    vector<vch *> packetsToSend;
    bool sending;

    vch receivedBytes;

public:
    ClientChannel(boost::asio::io_service &ioService_, boost::shared_ptr<tcp::socket> socket_)
        : ioService(ioService_), socket(socket_)
    {
        sending = false;
    }

    void sendResyncPacket()
    {
        packetsToSend.push_back(new vch);

        clearVchAndPackResyncPacket(packetsToSend.back());

        sendNextPacketIfNotBusy();
    }

    void sendFrameCmdsPacket(FrameEventsPacket fcp)
    {
        packetsToSend.push_back(new vch);

        clearVchAndPackFrameCmdsPacket(packetsToSend.back(), fcp);

        sendNextPacketIfNotBusy();
    }

    void sendNextPacketIfNotBusy()
    {
        if (!sending && packetsToSend.size() > 0)
        {
            sending = true;
            boost::asio::async_write(*socket,
                                     boost::asio::buffer(*(packetsToSend[0])),
                                     boost::bind(&ClientChannel::wrapUpSendingPacket,
                                                 this,
                                                 packetsToSend[0],
                                                 boost::asio::placeholders::error,
                                                 boost::asio::placeholders::bytes_transferred));
        }
    }

    void wrapUpSendingPacket(vch *sourceToDispose, const boost::system::error_code &error, size_t bytes_transferred)
    {
        if (error)
        {
            throw("ClientHandler error sending packet:" + error.value());
        }
        else
        {
            assert(packetsToSend[0] == sourceToDispose);

            delete packetsToSend[0];
            packetsToSend.erase(packetsToSend.begin());

            sending = false;

            sendNextPacketIfNotBusy();
        }
    }

    void clearVchAndReceiveNextCmd()
    {
        receivedBytes = vch(2);

        async_read(*socket,
                   boost::asio::buffer(receivedBytes),
                   boost::bind(&ClientChannel::sizeReceived,
                               this,
                               boost::asio::placeholders::error,
                               boost::asio::placeholders::bytes_transferred));
    }
    void sizeReceived(const boost::system::error_code &error, size_t transferred)
    {
        if (error)
        {
            throw runtime_error("Error receiving cmd size: " + error.value());
        }
        else
        {
            vchIter place = receivedBytes.begin();

            uint16_t size;
            unpackFromIter(place, "H", &size);

            clearVchAndReceiveCmdBody(size);
        }
    }
    void clearVchAndReceiveCmdBody(uint16_t size)
    {
        receivedBytes = vch(size);

        async_read(*socket,
                   boost::asio::buffer(receivedBytes),
                   boost::bind(&ClientChannel::cmdBodyReceived,
                               this,
                               boost::asio::placeholders::error,
                               boost::asio::placeholders::bytes_transferred));
    }
    void cmdBodyReceived(const boost::system::error_code &error, size_t transferred)
    {
        if (error)
        {
            throw runtime_error("Error receiving cmd body: " + error.value());
        }
        else
        {
            vchIter place = receivedBytes.begin();

            boost::shared_ptr<Cmd> cmd = unpackFullCmdAndMoveIter(&place);

            pendingCmds.push_back(cmd);

            clearVchAndReceiveNextCmd();
        }
    }
};

void Listener::handleAccept(boost::shared_ptr<tcp::socket> socket, const boost::system::error_code &error)
{
    if (error)
    {
        throw("Listener error accepting:" + error.value());
    }
    else
    {
        cout << "client connected!" << endl;
        clientChannels.push_back(new ClientChannel(ioService, socket));

        clientChannels.back()->sendResyncPacket();
        clientChannels.back()->clearVchAndReceiveNextCmd();
    }
    startAccept();
}

struct DepositEvent
{
    string userAddress;
    coinsInt amountInCoins;
    DepositEvent(string userAddress, coinsInt amountInCoins)
        : userAddress(userAddress), amountInCoins(amountInCoins) {}
    boost::shared_ptr<BalanceUpdate> toBalanceUpdateSharedPtr()
    {
        return boost::shared_ptr<BalanceUpdate>(new BalanceUpdate(userAddress, amountInCoins));
    }
};

vector<DepositEvent> pollPendingDeposits()
{
    vector<DepositEvent> depositEvents;

    boost::filesystem::path accountingDirPath("./accounting/pending_deposits/");
    boost::filesystem::directory_iterator directoryEndIter; // default constructor makes it an end_iter

    for (boost::filesystem::directory_iterator dirIter(accountingDirPath); dirIter != directoryEndIter; dirIter++)
    {
        if (boost::filesystem::is_regular_file(dirIter->path())) {
            string depositFilePath = dirIter->path().string();
            std::ifstream depositFile(depositFilePath);
            string depositData;

            if (depositFile.is_open())
            {
                while (depositFile)
                {
                    string depositCmdData;
                    getline(depositFile, depositCmdData);
                    if (depositCmdData.length() == 0)
                        continue;

                    vector<string> splitted;
                    boost::split(splitted, depositCmdData, boost::is_any_of(" "));
                    string userAddress = splitted[0];
                    string depositWeiString = splitted[1];
                    coinsInt depositInCoins = weiDepositStringToCoinsInt(depositWeiString);
                    
                    depositEvents.push_back(DepositEvent(userAddress, depositInCoins));
                }
            }
            else
            {
                throw runtime_error("Couldn't open a deposit file...\n");
            }
            depositFile.close();
            // delete the file, having processed it
            boost::filesystem::remove(dirIter->path());
        }
    }
    return depositEvents;
}

int main()
{
    game.testInit();

    boost::asio::io_service io_service;

    Listener listener(io_service);
    listener.startAccept();

    clock_t nextFrameStart = clock();

    // server will scan this directory for pending deposits (supplied by py/balance_tracker.py)
    boost::filesystem::path accountingDirPath("./accounting/pending_deposits/");
    boost::filesystem::directory_iterator directoryEndIter; // default constructor makes it an end_iter

    while (true)
    {
        // poll io_service, which will populate pendingCmds with anything the ClientChannels have received
        io_service.poll();

        // rate limit iteration to a maximum of SEC_PER_FRAME
        if (clock() < nextFrameStart)
            continue;
        nextFrameStart += (CLOCKS_PER_SEC * SEC_PER_FRAME);


        // let's count up deposits
        vector<boost::shared_ptr<BalanceUpdate>> pendingBalanceUpdates;
        // scan for any pending deposits
        vector<DepositEvent> depositEvents = pollPendingDeposits();
        for (uint i=0; i < depositEvents.size(); i++)
        {
            pendingBalanceUpdates.push_back(depositEvents[i].toBalanceUpdateSharedPtr());
        }

        // build FrameEventsPacket for this frame
        // includes all cmds we've received from clients since last time and all pending deposits
        FrameEventsPacket fcp(game.frame, pendingCmds, pendingBalanceUpdates);

        // send the packet out to all clients
        for (unsigned int i = 0; i < clientChannels.size(); i++)
        {
            clientChannels[i]->sendFrameCmdsPacket(fcp);
        }

        // execute all cmds on server-side game
        for (unsigned int i = 0; i < pendingCmds.size(); i++)
        {
            pendingCmds[i]->execute(&game);
        }
        pendingCmds.clear();

        // execute all balance updates
        for (unsigned int i = 0; i < pendingBalanceUpdates.size(); i++)
        {
            game.executeBalanceUpdate(pendingBalanceUpdates[i]);
        }
        pendingBalanceUpdates.clear();

        game.iterate();
    }

    cout << "oohhhhh Logan you done did it this time" << endl;

    return 0;
}
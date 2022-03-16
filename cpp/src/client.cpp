#include <GL/gl.h>
#include <GL/glu.h>
#include <stdio.h>
#include <iostream>
#include <boost/shared_ptr.hpp>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <vector>
#include <string>
#include "config.h"
#include "cmds.h"
#include "engine.h"
#include "graphics.h"
#include "common.h"
#include "input.h"
#include "packets.h"
#include "events.h"

using namespace std;
using namespace boost::asio::ip;

Game game;

UI ui;

vector<FrameEventsPacket> receivedFrameCmdsPackets;
vector<Game> receivedResyncs;

void clearVchAndBuildCmdPacket(vch *dest, boost::shared_ptr<Cmd> cmd)
{
    dest->clear();

    packTypechar(dest, cmd->getTypechar());
    cmd->pack(dest);

    vch prepended;
    packToVch(&prepended, "H", (uint16_t)(dest->size()));

    dest->insert(dest->begin(), prepended.begin(), prepended.end());
}

class ConnectionHandler
{
    vch receivedBytes;
    boost::asio::io_service &ioService;
    tcp::socket &socket;

    vector<vch *> packetsToSend;
    bool sending;

public:
    ConnectionHandler(boost::asio::io_service &ioService, tcp::socket &socket)
        : ioService(ioService), socket(socket)
    {
        sending = false;
    }
    string receiveSigChallenge()
    {
        boost::asio::streambuf buf(50);
        boost::asio::read(socket, buf);
        return string(boost::asio::buffer_cast<const char*>(buf.data()), buf.size());
    }
    void sendSignature(string sig)
    {
        boost::asio::write(socket, boost::asio::buffer(sig));
    }
    string receiveAddress()
    {
        boost::asio::streambuf buf(42);
        boost::asio::read(socket, buf);
        return string(boost::asio::buffer_cast<const char*>(buf.data()), buf.size());
    }
    void startReceivingLoop()
    {
        clearVchAndReceiveNextPacket();
    }

    void clearVchAndReceiveNextPacket()
    {
        receivedBytes = vch(9);

        async_read(socket,
                   boost::asio::buffer(receivedBytes),
                   boost::bind(&ConnectionHandler::typecharAndSizeReceived,
                               this,
                               boost::asio::placeholders::error,
                               boost::asio::placeholders::bytes_transferred));
    }
    void typecharAndSizeReceived(const boost::system::error_code &error, size_t received)
    {
        if (!error)
        {
            vchIter place = receivedBytes.begin();

            unsigned char packetTypechar;
            uint64_t size;
            unpackFromIter(place, "CQ", &packetTypechar, &size);

            switch (packetTypechar)
            {
            case PACKET_RESYNC_CHAR:
                clearVchAndReceiveResyncPacket(size);
                break;

            case PACKET_FRAMECMDS_CHAR:
                clearVchAndReceiveFrameCmdsPacket(size);
                break;
            }
        }
        else
        {
            cout << error.message() << endl;
            throw runtime_error("Error when receiving typechar");
        }
    }
    void clearVchAndReceiveResyncPacket(uint64_t size)
    {
        receivedBytes = vch(size);

        async_read(socket,
                   boost::asio::buffer(receivedBytes),
                   boost::bind(&ConnectionHandler::resyncPacketReceived,
                               this,
                               boost::asio::placeholders::error,
                               boost::asio::placeholders::bytes_transferred));
    }
    void clearVchAndReceiveFrameCmdsPacket(uint64_t size)
    {
        receivedBytes = vch(size);

        async_read(socket,
                   boost::asio::buffer(receivedBytes),
                   boost::bind(&ConnectionHandler::frameCmdsPacketReceived,
                               this,
                               boost::asio::placeholders::error,
                               boost::asio::placeholders::bytes_transferred));
    }
    void resyncPacketReceived(const boost::system::error_code &error, size_t received)
    {
        if (!error)
        {
            vchIter place = receivedBytes.begin();

            // cout << "BYTES:" << endl;
            // debugOutputVch(receivedBytes);
            // cout << endl << ":FIN" << endl;

            receivedResyncs.push_back(Game(&place));

            clearVchAndReceiveNextPacket();
        }
        else
        {
            throw runtime_error("Error when receiving resync packet");
        }
    }
    void frameCmdsPacketReceived(const boost::system::error_code &error, size_t received)
    {
        if (error)
        {
            throw runtime_error("Error receiving frameCmds packet:" + error.value());
        }
        else
        {
            vchIter place = receivedBytes.begin();

            receivedFrameCmdsPackets.push_back(FrameEventsPacket(&place));

            clearVchAndReceiveNextPacket();
        }
    }

    void sendCmd(boost::shared_ptr<Cmd> cmd)
    {
        packetsToSend.push_back(new vch);

        clearVchAndBuildCmdPacket(packetsToSend.back(), cmd);

        sendNextPacketIfNotBusy();
    }
    void sendNextPacketIfNotBusy()
    {
        if (!sending && packetsToSend.size() > 0)
        {
            sending = true;

            boost::asio::async_write(socket,
                                     boost::asio::buffer(*(packetsToSend[0])),
                                     boost::bind(&ConnectionHandler::wrapUpSendingPacket,
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
            throw runtime_error("ConnectionHandler error sending packet:" + error.value());
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
};

int main()
{
    boost::asio::io_service io_service;
    tcp::socket socket(io_service);

    cout << "Enter server IP (or 'l' for localhost) : ";
    string ipString;
    cin >> ipString;

    cout << "Connecting..." << endl;
    // this is "connecting" to INADDR_ANY, which is very weird, but translates (at least on linux) to
    // "connect to the loopback network". This is why this works on local tests.
    if (ipString == "l")
        socket.connect(tcp::endpoint(tcp::v4(), 8473));
    else
        socket.connect(tcp::endpoint(boost::asio::ip::address::from_string(ipString), 8473));

    // socket will now have its own local port.

    cout << "Creating ConnectionHandler..." << endl;

    ConnectionHandler connectionHandler(io_service, socket);

    // HANDSHAKE

    string playerAddress;
    int playerIdOrNegativeOne = -1;

    // Wait for the sig challenge and respond with the user's help
    string sigChallenge = connectionHandler.receiveSigChallenge();
    cout << "Sign this with the address you deposited to:" << endl << sigChallenge << endl;
    
    string userResponse;
    cout << "sig: ";
    cin >> userResponse;

    connectionHandler.sendSignature(userResponse + "\n");
    playerAddress = connectionHandler.receiveAddress();

    connectionHandler.startReceivingLoop();

    // Get the first resync packet
    while (true)
    {
        io_service.poll();

        if (receivedResyncs.size() > 0)
        {
            game = receivedResyncs[0];
            game.reassignEntityGamePointers();

            receivedResyncs.erase(receivedResyncs.begin());
            break;
        }
    }

    sf::RenderWindow* window = setupGraphics();
    
    ui = UI();

    ParticlesContainer particles;

    chrono::time_point<chrono::system_clock, chrono::duration<double>> nextFrameStart(chrono::system_clock::now());
    while (window->isOpen())
    {
        io_service.poll();

        chrono::time_point<chrono::system_clock, chrono::duration<double>> now(chrono::system_clock::now());
        if (now < nextFrameStart || receivedFrameCmdsPackets.size() == 0)
            continue;
        
        nextFrameStart += ONE_FRAME;

        vector<boost::shared_ptr<Cmd>> cmdsToSend = pollWindowEventsAndUpdateUI(game, &ui, playerIdOrNegativeOne, window);

        for (uint i=0; i < cmdsToSend.size(); i++)
        {
            cout << 1 << endl;
            if (!cmdsToSend[i])
                cout << "Uh oh, I'm seeing some null cmds in cmdsToSend!" << endl;
            else
                connectionHandler.sendCmd(cmdsToSend[i]);
        }
        cmdsToSend.clear();

        if (receivedResyncs.size() > 0 && receivedResyncs[0].frame == game.frame)
        {
            game = receivedResyncs[0];
            game.reassignEntityGamePointers();

            receivedResyncs.erase(receivedResyncs.begin());
        }

        FrameEventsPacket fcp = receivedFrameCmdsPackets[0];
        receivedFrameCmdsPackets.erase(receivedFrameCmdsPackets.begin());

        assert(fcp.frame == game.frame);

        // go through events
        for (unsigned int i = 0; i < fcp.events.size(); i++)
        {
            fcp.events[i]->execute(&game);
        }
        
        // go through cmds
        for (unsigned int i = 0; i < fcp.authdCmds.size(); i++)
        {
            if (auto unitCmd = boost::dynamic_pointer_cast<UnitCmd, Cmd>(fcp.authdCmds[i]->cmd))
            {
                unitCmd->executeAsPlayer(&game, fcp.authdCmds[i]->playerAddress);
            }
            else if (auto withdrawCmd = boost::dynamic_pointer_cast<WithdrawCmd, Cmd>(fcp.authdCmds[i]->cmd))
            {
                // ignore. Server processes withdrawals and creates an event.
            }
            else
            {
                cout << "Woah, I don't know how to handle that event as a client!" << endl;
            }
        }

        game.iterate();

        // Try to update playerId if necessary
        if (playerIdOrNegativeOne < 0)
        {
            playerIdOrNegativeOne = game.playerAddressToIdOrNegativeOne(playerAddress);
        }

        // check for game start cmd, and do some ux prep if we got one
        for (uint i=0; i<fcp.events.size(); i++)
        {
            if (auto gse = boost::dynamic_pointer_cast<HoneypotAddedEvent, Event>(fcp.events[i]))
            {
                if (playerIdOrNegativeOne >= 0)
                {
                    // find owned unit and center on it
                    for (uint i=0; i<game.entities.size(); i++)
                    {
                        if (auto unit = boost::dynamic_pointer_cast<Unit, Entity>(game.entities[i]))
                        {
                            if (unit->ownerId == playerIdOrNegativeOne)
                            {
                                ui.camera.gamePos = unit->pos;
                            }
                        }
                    }
                }
            }
        }

        // only display if we're not behind schedule
        now = chrono::system_clock::now();
        if (now <= nextFrameStart)
            display(window, &game, ui, &particles, game.playerAddressToIdOrNegativeOne(playerAddress));

        if (game.frame % 200 == 0)
            cout << "num ncps " << receivedFrameCmdsPackets.size() << endl;
    }
    delete window;

    cout << "Okay bye!" << endl;
    if (playerIdOrNegativeOne >= 0)
    {
        cout << "Withdrawing your " << game.players[playerIdOrNegativeOne].credit.getDollarString().toAnsiString() << " now..." << endl;
        connectionHandler.sendCmd(boost::shared_ptr<WithdrawCmd>(new WithdrawCmd(coinsInt(0))));
    } 

    socket.close();

    return 0;
}
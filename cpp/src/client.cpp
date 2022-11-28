#if defined(__APPLE__)
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#else
#include <GL/gl.h>
#include <GL/glu.h>
#endif
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
#include "netpack.h"

using namespace std;
using namespace boost::asio::ip;

using vch = vector<unsigned char>;

Game game;

extern sf::Font mainFont;

vector<FrameEventsPacket> receivedFrameEventsPackets;
vector<Game> receivedResyncs;

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
    Address receiveAddress()
    {
        boost::asio::streambuf buf(42);
        boost::asio::read(socket, buf);
        return Address(string(boost::asio::buffer_cast<const char*>(buf.data()), buf.size()));
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
            Netpack::Consumer typecharAndSizeData(receivedBytes.begin());

            uint64_t size = typecharAndSizeData.consumeUint64_t();

            uint8_t packetTypechar = typecharAndSizeData.consumeUint8_t();
            size -= 1; // since that included the packetTypechar we also just consumed

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
            Netpack::Consumer data(receivedBytes.begin());

            receivedResyncs.push_back(Game(&data));
            
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
            Netpack::Consumer data(receivedBytes.begin());

            receivedFrameEventsPackets.push_back(FrameEventsPacket(&data));

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

int main(int argc, char *argv[])
{
    uint latencyAllowance = 10;

    bool fullscreen = true;
    bool smallScreen = false;
    if (argc-1 == 1)
    {
        if (string(argv[1]) == "nofullscreen")
        {
            fullscreen = false;
        }
        else if (string(argv[1]) == "smallscreen")
        {
            fullscreen = false;
            smallScreen = true;
        }
    }
    boost::asio::io_service io_service;
    tcp::socket socket(io_service);

    cout << "Enter server IP ('l' for localhost, empty for default)" << endl << "IP: ";
    string ipString;
    getline(cin, ipString);

    cout << "Connecting..." << endl;

    if (ipString == "l")
        // this is "connecting" to INADDR_ANY, which is very weird, but translates (at least on linux) to
        // "connect to the loopback network". This is why this works on local tests.
        socket.connect(tcp::endpoint(tcp::v4(), MAIN_PORT));
    else if (ipString == "")
    {
        socket.connect(tcp::endpoint(boost::asio::ip::address::from_string(SERVER_IP_DEFAULT), MAIN_PORT));
    }
    else
        socket.connect(tcp::endpoint(boost::asio::ip::address::from_string(ipString), MAIN_PORT));

    // socket will now have its own local port.

    cout << "Creating ConnectionHandler..." << endl;

    ConnectionHandler connectionHandler(io_service, socket);

    // HANDSHAKE

    optional<uint8_t> maybePlayerId = {};

    // Wait for the sig challenge and respond with the user's help
    string sigChallenge = connectionHandler.receiveSigChallenge();
    cout << "Sign this with the address you deposited to:" << endl << sigChallenge << endl;

    string userResponse;
    cout << "sig: ";
    cin >> userResponse;

    connectionHandler.sendSignature(userResponse + "\n");
    Address playerAddress = connectionHandler.receiveAddress();

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

    sf::RenderWindow* window = setupGraphics(fullscreen, smallScreen);

    UI ui(mainFont);

    ParticlesContainer particles;

    Game replacedGameOnResync;

    chrono::time_point<chrono::system_clock, chrono::duration<double>> nextFrameStart(chrono::system_clock::now());
    while (window->isOpen())
    {
        io_service.poll();

        chrono::time_point<chrono::system_clock, chrono::duration<double>> now(chrono::system_clock::now());
        if (now < nextFrameStart)
            continue;

        // PROCESS INPUT
        vector<boost::shared_ptr<Cmd>> cmdsToSend = pollWindowEventsAndUpdateUI(&game, &ui, maybePlayerId, window, NULL);

        // ITERATE UI
        ui.iterate();
        if (ui.quitNow)
        {
            window->close();
        }

        // SEND CMDS
        for (unsigned int i=0; i < cmdsToSend.size(); i++)
        {
            if (!cmdsToSend[i])
                cout << "Uh oh, I'm seeing some null cmds in cmdsToSend!" << endl;
            else
                connectionHandler.sendCmd(cmdsToSend[i]); 
        }
        cmdsToSend.clear();

        // DISPLAY
        display(window, &game, ui, &particles, maybePlayerId, {}, true);

        if (game.frame % 200 == 0)
            cout << "Lag in frames: " << receivedFrameEventsPackets.size() << endl;
        
        // GAME LOGIC

        int framesToProcess;
        if (receivedFrameEventsPackets.size() == 0)
            framesToProcess = 0;
        else if (receivedFrameEventsPackets.size() < latencyAllowance)
            framesToProcess = 1;
        else
            framesToProcess = 2;
        
        for (int i=0; i<framesToProcess; i++)
        {
            // PROCESS AND CHECK RESYNCS
            if (receivedResyncs.size() > 0 && receivedResyncs[0].frame == game.frame)
            {
                // replacedGameOnResync = game;
                if (!gameStatesAreIdentical_triggerDebugIfNot(&game, &receivedResyncs[0]))
                {
                    cout << "Uh oh, I'm seeing some desyncs!" << endl;
                }
                // game = receivedResyncs[0];
                // game.reassignEntityGamePointers();

                receivedResyncs.erase(receivedResyncs.begin());
            }

            // PROCESS EVENTS FROM SERVER
            FrameEventsPacket fep = receivedFrameEventsPackets[0];
            receivedFrameEventsPackets.erase(receivedFrameEventsPackets.begin());

            assert(fep.frame == game.frame);

            for (unsigned int i = 0; i < fep.events.size(); i++)
            {
                fep.events[i]->execute(&game);
            }

            // EXECUTE USER CMDS FOR THIS FRAME
            for (unsigned int i = 0; i < fep.authdCmds.size(); i++)
            {
                auto cmd = fep.authdCmds[i]->cmd;
                if (auto unitCmd = boost::dynamic_pointer_cast<UnitCmd, Cmd>(cmd))
                {
                    unitCmd->executeAsPlayer(&game, fep.authdCmds[i]->playerAddress);
                }
                else if (auto spawnBeaconCmd = boost::dynamic_pointer_cast<SpawnBeaconCmd, Cmd>(cmd))
                {
                    spawnBeaconCmd->executeAsPlayer(&game, fep.authdCmds[i]->playerAddress);
                }
                else if (auto withdrawCmd = boost::dynamic_pointer_cast<WithdrawCmd, Cmd>(cmd))
                {
                    // ignore. Server processes withdrawals and creates an event.
                }
                else
                {
                    cout << "Woah, I don't know how to handle that cmd as a client!" << endl;
                }
            }

            // ITERATE GAME
            game.iterate();
        }

        nextFrameStart += ONE_FRAME;

        // UPDATE PLAYER ID
        if (!maybePlayerId)
        {
            maybePlayerId = game.playerAddressToMaybeId(playerAddress);
        }
    }
    delete window;

    cout << "Okay bye!" << endl;
    if (maybePlayerId)
    {
        cout << "Withdrawing your " << game.players[*maybePlayerId].credit.getDollarString().toAnsiString() << " now..." << endl;
        boost::shared_ptr<WithdrawCmd> withdrawCmd(new WithdrawCmd(coinsInt(0)));
        connectionHandler.sendCmd(withdrawCmd);
    }

    socket.close();

    return 0;
}

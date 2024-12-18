#include <stdio.h>
#include <iostream>
#include <boost/shared_ptr.hpp>
#include <vector>
#include <string>
#include <chrono>
#include <unistd.h>
#include "config.h"
#include "cmds.h"
#include "engine.h"
#include "graphics.h"
#include "common.h"
#include "input.h"
#include "events.h"
#include "packets.h"
#include "tutorial.h"
#include "client_networking.h"

sf::Font mainFont, tutorialFont;

void runLocal(GameSettings gameSettings, sf::RenderWindow* window, float totalCurrencyIn, bool checkNetpack);
void runTutorial(sf::RenderWindow* window);
void runClient(sf::RenderWindow* window, string serverIP, sf::Font* font);

enum MainMenuEvent
{
    StartLocal,
    StartLocalDebug,
    StartTutorial,
    StartClient,
    StartClientWithLocalhost,
    Quit
};

int main(int argc, char *argv[])
{
    // these are defaults which arguments may change
    bool fullscreen = true;
    bool smallScreen = false;
    bool showDevOptions = false;
    optional<string> customServerIP;

    int c;
    while ((c = getopt(argc, argv, "di:w::")) != -1)
    {
        switch (c)
        {
            case 'd':
                showDevOptions = true;
                break;
            case 'i':
                customServerIP = string(optarg);
                break;
            case 'w':
                fullscreen = false;
                if (optarg && string(optarg) == "small")
                {
                    smallScreen = true;
                }
                else
                {
                    smallScreen = false;
                }
                break;
        }
    }

    string serverIP = customServerIP ? *customServerIP : SERVER_IP_DEFAULT;

    loadFonts(&mainFont, &tutorialFont);
    loadIcons();

    sf::RenderWindow* window = setupGraphics(fullscreen, smallScreen);

    vector<tuple<string, MainMenuEvent>> menuOptions;

    menuOptions.push_back({"Local Playground / Demo", StartLocal});
    if (showDevOptions)
        menuOptions.push_back({"Local Debug", StartLocalDebug});
    menuOptions.push_back({"Tutorial", StartTutorial});
    menuOptions.push_back({"Connect to Server", StartClient});
    if (showDevOptions)
        menuOptions.push_back({"Connect to Local Server", StartClientWithLocalhost});
    menuOptions.push_back({"Quit", Quit});

    MainMenu mainMenu(window, menuOptions, &mainFont);

    chrono::time_point<chrono::system_clock, chrono::duration<double>> nextFrameStart(chrono::system_clock::now());

    while (window->isOpen())
    {
        chrono::time_point<chrono::system_clock, chrono::duration<double>> now(chrono::system_clock::now());
        if (now < nextFrameStart)
            continue;

        nextFrameStart += ONE_FRAME;

        optional<MainMenuEvent> menuEventMsg;

        sf::Event event;
        while (window->pollEvent(event))
        {
            switch (event.type)
            {
                case sf::Event::Closed:
                    window->close();
                    break;
                default:
                {
                    menuEventMsg = mainMenu.processEvent(event);
                }
            }

            if (menuEventMsg)
                break;
        }

        if (menuEventMsg)
        {
            switch (*menuEventMsg)
            {
                case StartLocal:
                {
                    runLocal(defaultGameSettings(), window, 10, false);
                    break;
                }
                case StartLocalDebug:
                {
                    runLocal(defaultGameSettings(), window, 10, true);
                    break;
                }
                case StartTutorial:
                {
                    runTutorial(window);
                    break;
                }
                case StartClient:
                {
                    runClient(window, serverIP, &mainFont);
                    break;
                }
                case StartClientWithLocalhost:
                {
                    runClient(window, "localhost", &mainFont);
                    break;
                }
                case Quit:
                {
                    window->close();
                    break;
                }
            }
        }

        window->setView(window->getDefaultView());
        window->clear();
        displayTitle(window, &mainFont);
        mainMenu.draw(window);
        window->display();
    }

    cleanupGraphics(window);
}

void runLocal(GameSettings gameSettings, sf::RenderWindow* window, float currencyInPerPlayer, bool checkNetpack) {
    coinsInt creditInPerPlayer = bcCurrencyAmountToCoinsIntND(currencyInPerPlayer);

    int randSeed = time(NULL);
    Game game(randSeed, time(NULL), gameSettings);

    vector<boost::shared_ptr<Event>> firstEvents;

    firstEvents.push_back(boost::shared_ptr<Event>(new DepositEvent(Address("0x0f0f00f000f00f00f000f00f00f000f00f00f000"), creditInPerPlayer)));
    firstEvents.push_back(boost::shared_ptr<Event>(new DepositEvent(Address("0xf00f00f000f00f00f000f00f00f000f00f00f000"), creditInPerPlayer)));
    firstEvents.push_back(boost::shared_ptr<Event>(new DepositEvent(Address("0x00ff00f000f00f00f000f00f00f000f00f00f000"), creditInPerPlayer)));
    firstEvents.push_back(boost::shared_ptr<Event>(new DepositEvent(Address("0xff0f00f000f00f00f000f00f00f000f00f00f000"), creditInPerPlayer)));
    firstEvents.push_back(boost::shared_ptr<Event>(new DepositEvent(Address("0xf0ff00f000f00f00f000f00f00f000f00f00f000"), creditInPerPlayer)));
    // firstEvents.push_back(boost::shared_ptr<Event>(new BalanceUpdateEvent(Address("0x0fff00f000f00f00f000f00f00f000f00f00f000"), creditInPerPlayer, true)));
    // firstEvents.push_back(boost::shared_ptr<Event>(new BalanceUpdateEvent(Address("0x888f00f000f00f00f000f00f00f000f00f00f000"), creditInPerPlayer, true)));
    // firstEvents.push_back(boost::shared_ptr<Event>(new BalanceUpdateEvent(Address("0x8fff00f000f00f00f000f00f00f000f00f00f000"), creditInPerPlayer, true)));
    // firstEvents.push_back(boost::shared_ptr<Event>(new BalanceUpdateEvent(Address("0xf88f00f000f00f00f000f00f00f000f00f00f000"), creditInPerPlayer, true)));
    // firstEvents.push_back(boost::shared_ptr<Event>(new BalanceUpdateEvent(Address("0x808f00f000f00f00f000f00f00f000f00f00f000"), creditInPerPlayer, true)));

    for (unsigned int i=0; i<firstEvents.size(); i++)
    {
        firstEvents[i]->execute(&game);
    }

    GameUI ui(&game.gameSettings, window, &mainFont, getSpriteForKeyButtonMsg, getSpriteForUnitTypechar, getUXView(), false);
    uint8_t currentPlayerId = 0;

    vector<boost::shared_ptr<Cmd>> pendingCmds;

    int lastDisplayedFrame = -1;

    chrono::time_point<chrono::system_clock, chrono::duration<double>> nextFrameStart(chrono::system_clock::now());

    while (window->isOpen()) {
        chrono::time_point<chrono::system_clock, chrono::duration<double>> now(chrono::system_clock::now());
        if (now < nextFrameStart)
        {
            if (lastDisplayedFrame < (int)game.frame)
            {
                lastDisplayedFrame = game.frame;

                // poll for cmds from input
                // (also updates GameUI)
                vector<boost::shared_ptr<Cmd>> newCmds = pollWindowEventsAndUpdateUI(&game, &ui, currentPlayerId, window, {}, getUXView());
                pendingCmds.insert(pendingCmds.begin(), newCmds.begin(), newCmds.end());

                // use ui.debugInt to switch playerIds
                uint8_t newPlayerId = ui.debugInt % game.players.size();
                if (newPlayerId != currentPlayerId)
                {
                    currentPlayerId = newPlayerId;
                    cout << "now controlling player " << currentPlayerId << endl;
                }

                display(window, &game, &ui, game.players[currentPlayerId].address, {}, &mainFont, &tutorialFont, false);
            }
        }
        else {
            nextFrameStart += ONE_FRAME;

            // depending on if we're testing netpack code or not, we have two different approaches.
            if (!checkNetpack)
            {
                // simply execute all pending cmds
                for (unsigned int i=0; i < pendingCmds.size(); i++)
                {
                    if (auto unitCmd = boost::dynamic_pointer_cast<UnitCmd, Cmd>(pendingCmds[i]))
                    {
                        unitCmd->executeAsPlayer(&game, game.playerIdToAddress(currentPlayerId));
                    }
                    else if (auto spawnBeaconCmd = boost::dynamic_pointer_cast<SpawnBeaconCmd, Cmd>(pendingCmds[i]))
                    {
                        spawnBeaconCmd->executeAsPlayer(&game, game.playerIdToAddress(currentPlayerId));
                    }
                    else if (auto withdrawCmd = boost::dynamic_pointer_cast<WithdrawCmd, Cmd>(pendingCmds[i]))
                    {
                        // ignore. Server processes withdrawals and creates an event.
                    }
                    else
                    {
                        cout << "Woah, I don't know how to handle that cmd!" << endl;
                    }
                }
                pendingCmds.clear();

                //iterate game
                game.iterate();
            }
            else
            {
                // gonna pack queued cmds up and clear list
                vector<vch*> packages;
                for (unsigned int i=0; i < pendingCmds.size(); i++)
                {
                    if (!pendingCmds[i])
                        cout << "Uh oh, I'm seeing some null cmds in cmdsToSend!" << endl;
                    else
                    {
                        packages.push_back(new vch);
                        clearVchAndBuildCmdPacket(packages.back(), pendingCmds[i]);
                    }
                }
                pendingCmds.clear();

                // now unpack them like the server does
                vector<boost::shared_ptr<AuthdCmd>> authdCmds;
                for (unsigned int i=0; i<packages.size(); i++)
                {
                    Netpack::Consumer source(packages[i]->begin() + 2); // we're looking past the size specifier, because in this case we already know...

                    optional<boost::shared_ptr<Cmd>> maybeCmd = consumeCmd(&source);
                    if (!maybeCmd)
                    {
                        throw runtime_error("Unrecognized command");
                    }
                    boost::shared_ptr<AuthdCmd> authdCmd = boost::shared_ptr<AuthdCmd>(new AuthdCmd(*maybeCmd, game.playerIdToAddress(currentPlayerId)));

                    authdCmds.push_back(authdCmd);

                    delete packages[i];
                }

                // now execute all authd cmds
                for (unsigned int i=0; i<authdCmds.size(); i++)
                {
                    auto cmd = authdCmds[i]->cmd;
                    if (auto unitCmd = boost::dynamic_pointer_cast<UnitCmd, Cmd>(cmd))
                    {
                        unitCmd->executeAsPlayer(&game, authdCmds[i]->playerAddress);
                    }
                    else if (auto spawnBeaconCmd = boost::dynamic_pointer_cast<SpawnBeaconCmd, Cmd>(cmd))
                    {
                        spawnBeaconCmd->executeAsPlayer(&game, authdCmds[i]->playerAddress);
                    }
                    else if (auto withdrawCmd = boost::dynamic_pointer_cast<WithdrawCmd, Cmd>(cmd))
                    {
                        // ignore. Server processes withdrawals and creates an event.
                    }
                    else
                    {
                        cout << "Woah, I don't know how to handle that cmd!" << endl;
                    }
                }

                // iterate, but sandwiched between some packing tests
                // cout << "PACKING GAME PRE-ITERATE" << endl;
                vch packData1;
                Netpack::Builder b(&packData1);
                // b.enableDebugOutput();
                game.pack(&b);

                // cout << "UNPACKING GAME" << endl;
                Netpack::Consumer c(packData1.begin());
                // c.enableDebugOutput();
                Game oldGameFromUnpack(&c);

                game.iterate();

                // cout << "PACKING GAME POST-ITERATE" << endl;
                vch packData2;
                b = Netpack::Builder(&packData2);
                // b.enableDebugOutput();
                game.pack(&b);

                // cout << "UNPACKING GAME" << endl;
                c = Netpack::Consumer(packData2.begin());
                // c.enableDebugOutput();
                Game newUnpackedGame(&c);

                assert(gameStatesAreIdentical_triggerDebugIfNot(&game, &newUnpackedGame));
            }

            ui.iterate();
            if (ui.quitNow)
            {
                return;
            }
        }
    }
}

void runTutorial(sf::RenderWindow* window)
{
    int randSeed = time(NULL);
    Game game(randSeed, time(NULL), defaultGameSettings());
    
    GameUI ui(&game.gameSettings, window, &mainFont, getSpriteForKeyButtonMsg, getSpriteForUnitTypechar, getUXView(), false);

    setupTutorialScenario(&game);

    Tutorial tutorial(&game, &ui);

    tutorial.start();

    vector<boost::shared_ptr<Cmd>> pendingCmds;

    int lastDisplayedFrame = -1;

    chrono::time_point<chrono::system_clock, chrono::duration<double>> nextFrameStart(chrono::system_clock::now());

    while (window->isOpen()) {
        chrono::time_point<chrono::system_clock, chrono::duration<double>> now(chrono::system_clock::now());
        if (now < nextFrameStart)
        {
            if (lastDisplayedFrame < (int)game.frame)
            {
                lastDisplayedFrame = game.frame;

                // poll for cmds from input
                // (also updates GameUI)
                vector<boost::shared_ptr<Cmd>> newCmds = pollWindowEventsAndUpdateUI(&game, &ui, 0, window, &tutorial, getUXView());
                pendingCmds.insert(pendingCmds.begin(), newCmds.begin(), newCmds.end());

                display(window, &game, &ui, Address(TUTORIAL_PLAYER_ADDRESS_STR), &tutorial, &mainFont, &tutorialFont, false);
            }
        }
        else
        {
            nextFrameStart += ONE_FRAME;

            // simply execute all pending cmds
            for (unsigned int i=0; i < pendingCmds.size(); i++)
            {
                if (auto unitCmd = boost::dynamic_pointer_cast<UnitCmd, Cmd>(pendingCmds[i]))
                {
                    unitCmd->executeAsPlayer(&game, game.playerIdToAddress(0));
                }
                else if (auto spawnBeaconCmd = boost::dynamic_pointer_cast<SpawnBeaconCmd, Cmd>(pendingCmds[i]))
                {
                    spawnBeaconCmd->executeAsPlayer(&game, game.playerIdToAddress(0));
                }
                else if (auto withdrawCmd = boost::dynamic_pointer_cast<WithdrawCmd, Cmd>(pendingCmds[i]))
                {
                    // ignore. Server processes withdrawals and creates an event.
                }
                else
                {
                    cout << "Woah, I don't know how to handle that cmd!" << endl;
                }
            }
            pendingCmds.clear();

            // iterate game
            game.iterate();

            ui.iterate();
            if (ui.quitNow)
            {
                return;
            }

            if (!tutorial.isFinished())
            {
                tutorial.update();
            }
        }
    }
}

using namespace boost::asio::ip;

optional<Address> runLoginScreen(sf::RenderWindow* window, ConnectionHandler* connectionHandler, string sigChallenge);

void runClient(sf::RenderWindow* window, string serverAddressString, sf::Font* font)
{
    unsigned int latencyAllowance = 10;

    boost::asio::io_service io_service;
    tcp::socket socket(io_service);

    try
    {
        if (serverAddressString == "localhost")
        {
            socket.connect(tcp::endpoint(tcp::v4(), MAIN_PORT));
        }
        else
        {
            socket.connect(tcp::endpoint(address::from_string(serverAddressString), MAIN_PORT));
        }
    }
    catch (const boost::wrapexcept<boost::system::system_error>& e)
    {
        string message =
            (e.code() == boost::asio::error::connection_refused) ?
            "Server is not respoding." :
            "Connection error: " + string(e.what());
        runNoticeWindow(window, message, font);
        return;
    }

    // socket will now have its own local port.

    ConnectionHandler connectionHandler(io_service, socket);

    // HANDSHAKE

    // Client initiates by sending code hash, for a version check.
    connectionHandler.sendCodeHash(GIT_COMMIT_HASH);

    // Server will send a success flag upon version match
    auto maybeSuccess = connectionHandler.receiveSuccessFlag();
    if (auto success = maybeSuccess)
    {
        if (!*success)
        {
            runNoticeWindow(window, "Version mismatch. Do you have the latest version of Coinfight?", &mainFont);
            return;
        }
    }
    else
    {
        runNoticeWindow(window, "Handshake failed! :/", &mainFont);
        return;
    }

    // Server will also immediately send a sigChallenge
    string sigChallenge = connectionHandler.receiveSigChallenge();

    optional<Address> maybePlayerAddress = runLoginScreen(window, &connectionHandler, sigChallenge);
    if (!maybePlayerAddress)
    {
        // Back was pressed; return
        return;
    }
    Address playerAddress = *maybePlayerAddress;

    connectionHandler.startReceivingLoop();

    Game game(0, 0, GameSettings()); // will be overwritten soon with the resync packet
    
    optional<uint8_t> maybePlayerId = {};

    // Get the first resync packet
    while (true)
    {
        io_service.poll();

        if (connectionHandler.receivedResyncs.size() > 0)
        {
            game = connectionHandler.receivedResyncs[0];
            game.reassignEntityGamePointers();

            connectionHandler.receivedResyncs.erase(connectionHandler.receivedResyncs.begin());
            break;
        }
    }

    GameUI ui(&game.gameSettings, window, &mainFont, getSpriteForKeyButtonMsg, getSpriteForUnitTypechar, getUXView(), true);

    chrono::time_point<chrono::system_clock, chrono::duration<double>> nextFrameStart(chrono::system_clock::now());
    while (window->isOpen())
    {
        io_service.poll();

        chrono::time_point<chrono::system_clock, chrono::duration<double>> now(chrono::system_clock::now());
        if (now < nextFrameStart)
            continue;

        // PROCESS INPUT
        vector<boost::shared_ptr<Cmd>> cmdsToSend = pollWindowEventsAndUpdateUI(&game, &ui, maybePlayerId, window, NULL, getUXView());

        // SEND CMDS
        for (unsigned int i=0; i < cmdsToSend.size(); i++)
        {
            if (!cmdsToSend[i])
                cout << "Uh oh, I'm seeing some null cmds in cmdsToSend!" << endl;
            else
                connectionHandler.sendCmd(cmdsToSend[i]);
        }
        cmdsToSend.clear();

        // ITERATE GAME_UI
        ui.iterate();
        if (ui.quitNow)
        {
            return;
        }

        // DISPLAY
        display(window, &game, &ui, playerAddress, {}, &mainFont, &tutorialFont, true);

        if (game.frame % 200 == 0)
            cout << "Latency buffer size: " << connectionHandler.receivedFrameEventsPackets.size() << endl;

        // GAME LOGIC

        int framesToProcess;
        if (connectionHandler.receivedFrameEventsPackets.size() == 0)
            framesToProcess = 0;
        else if (connectionHandler.receivedFrameEventsPackets.size() < latencyAllowance)
            framesToProcess = 1;
        else
            framesToProcess = 2;

        for (int i=0; i<framesToProcess; i++)
        {
            // PROCESS AND CHECK RESYNCS
            if (connectionHandler.receivedResyncs.size() > 0 && connectionHandler.receivedResyncs[0].frame == game.frame)
            {
                // replacedGameOnResync = game;
                if (!gameStatesAreIdentical_triggerDebugIfNot(&game, &connectionHandler.receivedResyncs[0]))
                {
                    cout << "Uh oh, I'm seeing some desyncs!" << endl;
                }
                // game = receivedResyncs[0];
                // game.reassignEntityGamePointers();

                connectionHandler.receivedResyncs.erase(connectionHandler.receivedResyncs.begin());
            }

            // PROCESS EVENTS FROM SERVER
            FrameEventsPacket fep = connectionHandler.receivedFrameEventsPackets[0];
            connectionHandler.receivedFrameEventsPackets.erase(connectionHandler.receivedFrameEventsPackets.begin());

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

    socket.close();
}

optional<Address> runLoginScreen(sf::RenderWindow* mainWindow, ConnectionHandler* connectionHandler, string sigChallenge)
{
    vector2i center = getScreenSize(mainWindow) / 2;

    LoginWindow loginWindow(center, sigChallenge, &mainFont);

    while (mainWindow->isOpen())
    {
        sf::Event event;
        optional<LoginWindow::Msg> msg = {};
        optional<tuple<Address, string>> addressAndSigResponse;
        while (mainWindow->pollEvent(event))
        {
            msg = loginWindow.processEvent(event);
            if (msg) switch (*msg)
            {
                case LoginWindow::Back:
                {
                    return {};
                }
                case LoginWindow::ResponseEntered:
                {
                    if (!loginWindow.addressAndSigResponse)
                    {
                        throw runtime_error("LoginWindow says it got a addressAndSigResponse, but there isn't anything here!");
                    }
                    addressAndSigResponse = loginWindow.addressAndSigResponse;
                }
            }
        }

        if (addressAndSigResponse)
        {
            Address taggedAddress = get<0>(*addressAndSigResponse);
            string sig = get<1>(*addressAndSigResponse);
            connectionHandler->sendSignature(sig);
            optional<Address> maybePlayerAddress = connectionHandler->receiveAddressIfNotDenied();
            if (maybePlayerAddress)
            {
                if (taggedAddress != *maybePlayerAddress)
                {
                    runNoticeWindow(mainWindow, "The address claimed (" + taggedAddress.getString() + ") doesn't match the address derived from the signature (" + maybePlayerAddress->getString() + "). Maybe you signed an old or misformed challenge?", &mainFont);
                    return {};
                }
                return maybePlayerAddress;
            }
            else
            {
                runNoticeWindow(mainWindow, "The server received the signature, but the signature was invalid.", &mainFont);
                return {};
            }
        }

        mainWindow->clear();
        loginWindow.draw(mainWindow);
        mainWindow->display();
    }

    return {};
}

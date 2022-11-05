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

Game game;

UI ui;

int main(int argc, char *argv[])
{
    unsigned int playerStartDollars = 10;
    unsigned int honeypotStartingDollars = 50;
    bool fullscreen = true;
    bool smallScreen = false;

    bool startTutorial = true; // may be switched when processing arguments

    int c;
    while ((c = getopt(argc, argv, "s:g:w::")) != -1)
    {
        switch (c)
        {
            case 's':
                playerStartDollars = stoi(optarg);
                startTutorial = false;
                break;
            case 'g':
                honeypotStartingDollars = stoi(optarg);
                startTutorial = false;
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

    coinsInt honeypotStartingAmount = dollarsToCoinsIntND(honeypotStartingDollars);
    coinsInt playerStartCredit = dollarsToCoinsIntND(playerStartDollars);

    game = Game();

    vector<boost::shared_ptr<Event>> firstEvents;

    if (startTutorial)
    {
        firstEvents.push_back(boost::shared_ptr<Event>(new BalanceUpdateEvent(Address("0xf00f00f000f00f00f000f00f00f000f00f00f000"), dollarsToCoinsIntND(4.5), true)));

        boost::shared_ptr<GoldPile> gp = boost::shared_ptr<GoldPile>(new GoldPile(vector2fp()));
        gp->gold.createMoreByFiat(dollarsToCoinsIntND(6));
        game.registerNewEntityIgnoringCollision(gp);
    }
    else
    {
        firstEvents.push_back(boost::shared_ptr<Event>(new BalanceUpdateEvent(Address("0xf00f00f000f00f00f000f00f00f000f00f00f000"), playerStartCredit, true)));
        firstEvents.push_back(boost::shared_ptr<Event>(new BalanceUpdateEvent(Address("0x0f0f00f000f00f00f000f00f00f000f00f00f000"), playerStartCredit, true)));
        firstEvents.push_back(boost::shared_ptr<Event>(new BalanceUpdateEvent(Address("0x00ff00f000f00f00f000f00f00f000f00f00f000"), playerStartCredit, true)));
        firstEvents.push_back(boost::shared_ptr<Event>(new HoneypotAddedEvent(honeypotStartingAmount)));
    }

    for (unsigned int i=0; i<firstEvents.size(); i++)
    {
        firstEvents[i]->execute(&game);
    }

    sf::RenderWindow* window = setupGraphics(fullscreen, smallScreen);

    ui = UI();
    uint8_t currentPlayerId = 0;

    optional<Tutorial> tutorial;
    if (startTutorial)
    {
        tutorial = {Tutorial(&game, &ui)};
        tutorial->start(&game, &ui);
    }

    vector<boost::shared_ptr<Cmd>> pendingCmdsToSend;

    ParticlesContainer particles;

    int lastDisplayedFrame = -1;

    chrono::time_point<chrono::system_clock, chrono::duration<double>> nextFrameStart(chrono::system_clock::now());
    while (window->isOpen())
    {
        chrono::time_point<chrono::system_clock, chrono::duration<double>> now(chrono::system_clock::now());
        if (now < nextFrameStart)
        {
            if (lastDisplayedFrame < (int)game.frame)
            {
                lastDisplayedFrame = game.frame;
                // if we have time, display and perform UX.

                // poll for cmds from input
                // (also updates UI)
                vector<boost::shared_ptr<Cmd>> newCmds = pollWindowEventsAndUpdateUI(&game, &ui, currentPlayerId, window);
                pendingCmdsToSend.insert(pendingCmdsToSend.begin(), newCmds.begin(), newCmds.end());

                // use ui.debugInt to switch playerIds
                uint8_t newPlayerId = ui.debugInt % game.players.size();
                if (newPlayerId != currentPlayerId)
                {
                    currentPlayerId = newPlayerId;
                    cout << "now controlling player " << currentPlayerId << endl;
                }

                display(window, &game, ui, &particles, currentPlayerId, tutorial, false);
            }
        }
        else {
            nextFrameStart += ONE_FRAME;

            // gonna pack queued cmds up and clear list
            vector<vch*> packages;
            for (unsigned int i=0; i < pendingCmdsToSend.size(); i++)
            {
                if (!pendingCmdsToSend[i])
                    cout << "Uh oh, I'm seeing some null cmds in cmdsToSend!" << endl;
                else
                {
                    packages.push_back(new vch);
                    clearVchAndBuildCmdPacket(packages.back(), pendingCmdsToSend[i]);
                }
            }
            pendingCmdsToSend.clear();

            // now unpack them like the server does
            vector<boost::shared_ptr<AuthdCmd>> authdCmds;
            for (unsigned int i=0; i<packages.size(); i++)
            {
                Netpack::Consumer source(packages[i]->begin() + 2); // we're looking past the size specifier, because in this case we already know...

                boost::shared_ptr<Cmd> cmd = consumeCmd(&source);
                boost::shared_ptr<AuthdCmd> authdCmd = boost::shared_ptr<AuthdCmd>(new AuthdCmd(cmd, game.playerIdToAddress(currentPlayerId)));

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

            ui.iterate();
            if (ui.quitNow)
            {
                window->close();
            }

            if (tutorial && !tutorial->isFinished())
            {
                tutorial->update(&game, &ui);
            }
        }
    }
}

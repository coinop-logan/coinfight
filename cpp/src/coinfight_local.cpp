#include <GL/gl.h>
#include <GL/glu.h>
#include <stdio.h>
#include <iostream>
#include <boost/shared_ptr.hpp>
#include <vector>
#include <string>
#include <chrono>
#include "config.h"
#include "cmds.h"
#include "engine.h"
#include "graphics.h"
#include "common.h"
#include "input.h"
#include "events.h"
#include "packets.h"

Game game;

UI ui;

void clearVchAndBuildCmdPacket(vch *dest, boost::shared_ptr<Cmd> cmd)
{
    dest->clear();

    packTypechar(dest, cmd->getTypechar());
    cmd->pack(dest);

    vch prepended;
    packToVch(&prepended, "H", (uint16_t)(dest->size()));

    dest->insert(dest->begin(), prepended.begin(), prepended.end());
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

int main(int argc, char *argv[])
{
    if (argc-1 == 0)
    {
        cout << "Need an argument for how much money to put in for the honeypot!" << endl;
        return 1;
    }
    int honeypotStartingDollars = stoi(argv[1]);
    coinsInt honeypotStartingAmount = dollarsToCoinsInt(honeypotStartingDollars);

    game = Game();

    vector<boost::shared_ptr<Event>> firstEvents;

    firstEvents.push_back(boost::shared_ptr<Event>(new BalanceUpdateEvent("0xf00", 15000, true)));
    firstEvents.push_back(boost::shared_ptr<Event>(new BalanceUpdateEvent("0x0f0", 15000, true)));
    firstEvents.push_back(boost::shared_ptr<Event>(new GameStartEvent(honeypotStartingAmount)));
    
    for (uint i=0; i<firstEvents.size(); i++)
    {
        firstEvents[i]->execute(&game);
    }

    sf::RenderWindow* window = setupGraphics();

    ui = UI();
    uint currentPlayerId = 0;

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
                vector<boost::shared_ptr<Cmd>> newCmds = pollWindowEventsAndUpdateUI(game, &ui, currentPlayerId, window);
                pendingCmdsToSend.insert(pendingCmdsToSend.begin(), newCmds.begin(), newCmds.end());

                // use ui.debugInt to switch playerIds
                int newPlayerId = ui.debugInt % game.players.size();
                if (newPlayerId != currentPlayerId)
                {
                    currentPlayerId = newPlayerId;
                    cout << "now controlling player " << currentPlayerId << endl;
                }

                display(window, &game, ui, &particles, currentPlayerId);
            }
        }
        else {
            nextFrameStart += ONE_FRAME;

            // gonna pack queued cmds up and clear list
            vector<vch*> packages;
            for (uint i=0; i < pendingCmdsToSend.size(); i++)
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
            for (uint i=0; i<packages.size(); i++)
            {
                vchIter place = packages[i]->begin() + 2; // we're looking past the size specifier, because in this case we already know...

                boost::shared_ptr<Cmd> cmd = unpackFullCmdAndMoveIter(&place);
                boost::shared_ptr<AuthdCmd> authdCmd = boost::shared_ptr<AuthdCmd>(new AuthdCmd(cmd, game.playerIdToAddress(currentPlayerId)));

                authdCmds.push_back(authdCmd);

                delete packages[i];
            }

            // now execute all authd cmds
            for (uint i=0; i<authdCmds.size(); i++)
            {
                if (auto unitCmd = boost::dynamic_pointer_cast<UnitCmd, Cmd>(authdCmds[i]->cmd))
                {
                    unitCmd->executeAsPlayer(&game, authdCmds[i]->playerAddress);
                }
                else if (auto withdrawCmd = boost::dynamic_pointer_cast<WithdrawCmd, Cmd>(authdCmds[i]->cmd))
                {
                    // ignore for the purposes of this local binary
                }
            }

            game.iterate();
        }
    }
}
#include <GL/gl.h>
#include <GL/glu.h>
#include <stdio.h>
#include <iostream>
#include <boost/shared_ptr.hpp>
#include <vector>
#include <string>
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

uint playerId;

int main(int argc, char *argv[])
{
    if (argc-1 == 0)
    {
        cout << "Need an argument for how much money to put in for the honeypot!" << endl;
        return 1;
    }

    game = Game();

    vector<boost::shared_ptr<Event>> firstEvents;

    firstEvents.push_back(boost::shared_ptr<Event>(new BalanceUpdateEvent("0xf00", 10000, true)));
    firstEvents.push_back(boost::shared_ptr<Event>(new BalanceUpdateEvent("0x0f0", 10000, true)));
    
    for (uint i=0; i<firstEvents.size(); i++)
    {
        firstEvents[i]->execute(&game);
    }

    game.startMatchOrPrintError();

    sf::RenderWindow* window = setupGraphics();

    clock_t nextFrameStart = clock() + (CLOCKS_PER_SEC * SEC_PER_FRAME);

    ui = UI();
    uint currentPlayerId = 0;

    while (true)
    {
        if (clock() < nextFrameStart)
            continue;
        
        nextFrameStart += (CLOCKS_PER_SEC * SEC_PER_FRAME);

        // poll for cmds from input
        // (also updates UI)
        vector<boost::shared_ptr<Cmd>> cmdsToSend = pollWindowEvents(game, &ui, window);

        // use ui.debugInt to switch playerIds
        int newPlayerId = ui.debugInt % game.players.size();
        if (newPlayerId != currentPlayerId)
        {
            currentPlayerId = newPlayerId;
            cout << "now controlling player " << currentPlayerId << endl;
        }

        // gonna pack all these up
        vector<vch*> packages;
        for (uint i=0; i < cmdsToSend.size(); i++)
        {
            if (!cmdsToSend[i])
                cout << "Uh oh, I'm seeing some null cmds in cmdsToSend!" << endl;
            else
            {
                packages.push_back(new vch);
                clearVchAndBuildCmdPacket(packages.back(), cmdsToSend[i]);
            }
        }

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
            authdCmds[i]->execute(&game);
        }

        game.iterate();

        display(window, &game, ui, currentPlayerId);
    }
}
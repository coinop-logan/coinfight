#include <iostream>
#include <boost/shared_ptr.hpp>
#include "events.h"
#include "common.h"
#include "vchpack.h"
#include "config.h"

using namespace std;

boost::shared_ptr<Event> unpackFullEventAndMoveIter(vchIter *iter)
{
    unsigned char typechar;
    *iter = unpackTypecharFromIter(*iter, &typechar);

    switch (typechar)
    {
    case NULL_TYPECHAR:
        return boost::shared_ptr<Event>();
        break;
    case EVENT_BALANCEUPDATE_CHAR:
        return boost::shared_ptr<Event>(new BalanceUpdateEvent(iter));
        break;
    case EVENT_GAMESTART_CHAR:
        return boost::shared_ptr<Event>(new GameStartEvent(iter));
        break;
    default:
        throw runtime_error("Trying to unpack an unrecognized event");
    }
}

unsigned char Event::typechar()
{
    throw runtime_error("typechar() has not been defined for an event.");
}
void Event::pack(vch *dest)
{
    throw runtime_error("pack() has not been defined for an event.");
}
void Event::unpackAndMoveIter(vchIter *iter)
{
    throw runtime_error("unpackAndMoveIter() has not been defined for an event.");
}
void Event::execute(Game *game)
{
    throw runtime_error("execute() has not been defined for an event.");
}

void Event::packEvent(vch *dest) {}

void Event::unpackEventAndMoveIter(vchIter *iter) {}

Event::Event() {}

Event::Event(vchIter *iter)
{
    unpackEventAndMoveIter(iter);
}

unsigned char BalanceUpdateEvent::typechar()
{
    return EVENT_BALANCEUPDATE_CHAR;
}
void BalanceUpdateEvent::execute(Game *game)
{
    if (isDeposit)
    {
        // see if player exists for this address
        bool found = false;
        int playerId;
        for (uint i=0; i<game->players.size(); i++)
        {
            if (game->players[i].address == userAddress)
            {
                found = true;
                playerId = i;
            }
        }
        // if no user for this address, create one
        if (!found)
        {
            game->players.push_back(Player(userAddress));
            playerId = game->players.size() - 1;
        }

        game->players[playerId].credit.createMoreByFiat(amount);
    }
    else
    {
        throw runtime_error("woah, we haven't done withdrawals yet!!");
    }
}
void BalanceUpdateEvent::pack(vch *dest)
{
    packEvent(dest);

    packStringToVch(dest, userAddress);
    packToVch(dest, "L", amount);
    packToVch(dest, "C", (unsigned char)isDeposit);
}
void BalanceUpdateEvent::unpackAndMoveIter(vchIter *iter)
{
    *iter = unpackStringFromIter(*iter, 50, &userAddress);
    cout << "userAddress after unpack: " << userAddress << endl;
    *iter = unpackFromIter(*iter, "L", &amount);
    unsigned char isDepositBoolChar;
    *iter = unpackFromIter(*iter, "C", &isDepositBoolChar);
    isDeposit = (bool)isDepositBoolChar;
}

BalanceUpdateEvent::BalanceUpdateEvent(string userAddress, coinsInt amount, bool isDeposit)
    : Event(),
      userAddress(userAddress), amount(amount), isDeposit(isDeposit) {}

BalanceUpdateEvent::BalanceUpdateEvent(vchIter *iter)
    : Event(iter)
{
    unpackAndMoveIter(iter);
}

unsigned char GameStartEvent::typechar()
{
    return EVENT_GAMESTART_CHAR;
}

void GameStartEvent::execute(Game *game)
{
    boost::shared_ptr<GoldPile> honeypotGoldPile = boost::shared_ptr<GoldPile>(new GoldPile(game, game->getNextEntityRef(), vector2f(0,0)));
    honeypotGoldPile->gold.createMoreByFiat(honeypotAmount);
    game->entities.push_back(honeypotGoldPile);

    game->startMatchOrPrintError();
}

void GameStartEvent::pack(vch *dest)
{
    packToVch(dest, "L", honeypotAmount);
    packEvent(dest);
}
void GameStartEvent::unpackAndMoveIter(vchIter *iter)
{
    *iter = unpackFromIter(*iter, "L", &honeypotAmount);
}

GameStartEvent::GameStartEvent(coinsInt honeypotAmount)
    : Event(), honeypotAmount(honeypotAmount)
    {}
GameStartEvent::GameStartEvent(vchIter *iter)
    : Event(iter)
{
    unpackAndMoveIter(iter);
}
#ifndef EVENTS_H
#define EVENTS_H

#include <string>
#include "vchpack.h"
#include "coins.h"
#include "engine.h"

using namespace std;

struct Event;

boost::shared_ptr<Event> unpackFullEventAndMoveIter(vchIter *iter);

struct Event
{
    virtual unsigned char typechar();
    virtual void pack(vch *dest);
    virtual void unpackAndMoveIter(vchIter *iter);
    virtual void execute(Game *game);

    void packEvent(vch *dest);
    void unpackEventAndMoveIter(vchIter *iter);

    Event();
    Event(vchIter *iter);
};

struct BalanceUpdateEvent : public Event
{
    string userAddress;
    coinsInt amount;
    bool isDeposit;

    unsigned char typechar();

    void execute(Game *game);

    void pack(vch *dest);
    void unpackAndMoveIter(vchIter *iter);

    BalanceUpdateEvent(string userAddress, coinsInt amount, bool isDeposit);
    BalanceUpdateEvent(vchIter *iter);
};

struct GameStartEvent : public Event
{
    coinsInt honeypotAmount;

    unsigned char typechar();

    void execute(Game *game);

    void pack(vch *dest);
    void unpackAndMoveIter(vchIter *iter);

    GameStartEvent(coinsInt honeypotAmount);
    GameStartEvent(vchIter *iter);
};

#endif // EVENTS_H
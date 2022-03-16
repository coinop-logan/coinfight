#ifndef EVENTS_H
#define EVENTS_H

#include <string>
#include "vchpack.h"
#include "coins.h"
#include "engine.h"

using namespace std;

const unsigned char EVENT_BALANCEUPDATE_CHAR = 1;
const unsigned char EVENT_HONEYPOT_CHAR = 2;

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

struct HoneypotAddedEvent : public Event
{
    coinsInt honeypotAmount;

    unsigned char typechar();

    void execute(Game *game);

    void pack(vch *dest);
    void unpackAndMoveIter(vchIter *iter);

    HoneypotAddedEvent(coinsInt honeypotAmount);
    HoneypotAddedEvent(vchIter *iter);
};

#endif // EVENTS_H
#ifndef EVENTS_H
#define EVENTS_H

#include <string>
#include "coins.h"
#include "engine.h"
#include "netpack.h"

using namespace std;

const unsigned char EVENT_BALANCEUPDATE_CHAR = 1;
const unsigned char EVENT_HONEYPOT_CHAR = 2;

struct Event;

boost::shared_ptr<Event> consumeEvent(Netpack::Consumer*);

struct Event
{
    virtual unsigned char typechar();
    virtual void execute(Game *game);

    void packEventBasics(Netpack::Builder*);
    virtual void pack(Netpack::Builder*);

    Event();
    Event(Netpack::Consumer*);
};

struct BalanceUpdateEvent : public Event
{
    string userAddress;
    coinsInt amount;
    bool isDeposit;

    unsigned char typechar();

    void execute(Game *game);

    void pack(Netpack::Builder*);

    BalanceUpdateEvent(string userAddress, coinsInt amount, bool isDeposit);
    BalanceUpdateEvent(Netpack::Consumer*);
};

struct HoneypotAddedEvent : public Event
{
    coinsInt honeypotAmount;

    unsigned char typechar();

    void execute(Game *game);

    void pack(Netpack::Builder*);

    HoneypotAddedEvent(coinsInt honeypotAmount);
    HoneypotAddedEvent(Netpack::Consumer*);
};

#endif // EVENTS_H
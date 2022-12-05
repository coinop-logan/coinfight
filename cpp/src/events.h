#ifndef EVENTS_H
#define EVENTS_H

#include <string>
#include "coins.h"
#include "engine.h"
#include "netpack.h"

using namespace std;

const uint8_t EVENT_BALANCEUPDATE_CHAR = 1;
const uint8_t EVENT_HONEYPOT_CHAR = 2;
const uint8_t EVENT_RESETBEACONS_CHAR = 3;

struct Event;

boost::shared_ptr<Event> consumeEvent(Netpack::Consumer*);

struct Event
{
    virtual uint8_t typechar();
    virtual void execute(Game *game);

    void packEventBasics(Netpack::Builder*);
    virtual void pack(Netpack::Builder*);

    Event();
    Event(Netpack::Consumer*);
};

struct BalanceUpdateEvent : public Event
{
    Address userAddress;
    coinsInt amount;
    bool isDeposit;

    uint8_t typechar();

    void execute(Game *game);

    void pack(Netpack::Builder*);

    BalanceUpdateEvent(Address userAddress, coinsInt amount, bool isDeposit);
    BalanceUpdateEvent(Netpack::Consumer*);
};

struct HoneypotAddedEvent : public Event
{
    coinsInt honeypotAmount;

    uint8_t typechar();

    void execute(Game *game);

    void pack(Netpack::Builder*);

    HoneypotAddedEvent(coinsInt honeypotAmount);
    HoneypotAddedEvent(Netpack::Consumer*);
};

struct ResetBeaconsEvent : public Event
{
    uint8_t typechar();

    void execute(Game *game);

    void pack(Netpack::Builder*);

    ResetBeaconsEvent();
    ResetBeaconsEvent(Netpack::Consumer*);
};

#endif // EVENTS_H
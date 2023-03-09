#ifndef EVENTS_H
#define EVENTS_H

#include <string>
#include "coins.h"
#include "engine.h"
#include "netpack.h"

using namespace std;

const uint8_t EVENT_DEPOSIT_CHAR = 1;
const uint8_t EVENT_WITHDRAW_CHAR = 2;
const uint8_t EVENT_HONEYPOT_CHAR = 3;
const uint8_t EVENT_RESETBEACONS_CHAR = 4;

struct Event;

optional<boost::shared_ptr<Event>> consumeEvent(Netpack::Consumer*);

struct Event
{
    virtual uint8_t typechar();
    virtual void execute(Game *game);

    void packEventBasics(Netpack::Builder*);
    virtual void pack(Netpack::Builder*);

    Event();
    Event(Netpack::Consumer*);
};

struct WithdrawEvent : public Event
{
    Address userAddress;
    coinsInt amount;

    WithdrawEvent(Address userAddress, coinsInt amount);
    
    uint8_t typechar() { return EVENT_WITHDRAW_CHAR; }

    void execute(Game *game);

    void pack(Netpack::Builder*);

    WithdrawEvent(Netpack::Consumer*);
};

struct DepositEvent : public Event
{
    Address userAddress;
    coinsInt amount;

    DepositEvent(Address userAddress, coinsInt amount);
    
    uint8_t typechar() { return EVENT_DEPOSIT_CHAR; }

    void execute(Game *game);

    void pack(Netpack::Builder*);

    DepositEvent(Netpack::Consumer*);
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
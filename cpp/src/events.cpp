#include <iostream>
#include <boost/shared_ptr.hpp>
#include "events.h"
#include "common.h"
#include "netpack.h"
#include "config.h"

using namespace std;

optional<boost::shared_ptr<Event>> consumeEvent(Netpack::Consumer* from)
{
    uint8_t typechar = consumeTypechar(from);

    switch (typechar)
    {
    case NULL_TYPECHAR:
        return boost::shared_ptr<Event>();
    case EVENT_WITHDRAW_CHAR:
        return boost::shared_ptr<Event>(new WithdrawEvent(from));
    case EVENT_DEPOSIT_CHAR:
        return boost::shared_ptr<Event>(new DepositEvent(from));
    case EVENT_HONEYPOT_CHAR:
        return boost::shared_ptr<Event>(new HoneypotAddedEvent(from));
    case EVENT_RESETBEACONS_CHAR:
        return boost::shared_ptr<Event>(new ResetBeaconsEvent(from));
    }
    
    return {};
}

uint8_t Event::typechar()
{
    throw runtime_error("typechar() has not been defined for an event.");
}
void Event::pack(Netpack::Builder*)
{
    throw runtime_error("pack() has not been defined for an event.");
}
void Event::execute(Game *game)
{
    throw runtime_error("execute() has not been defined for an event.");
}


Event::Event() {}
void Event::packEventBasics(Netpack::Builder* to)
{
    packTypechar(to, typechar());
}
Event::Event(Netpack::Consumer* from) {}

WithdrawEvent::WithdrawEvent(Address userAddress, coinsInt amount)
    : Event(),
      userAddress(userAddress), amount(amount)
      {}
    
void WithdrawEvent::execute(Game *game)
{
    if (auto maybePlayerId = game->playerAddressToMaybeId(userAddress))
    {
        game->players[*maybePlayerId].credit.destroySomeByFiat(amount);
    }
    else
    {
        cout << "Woah, we're executing a withdrawal for an address I can't find!" << endl;
    }
}

void WithdrawEvent::pack(Netpack::Builder* to)
{
    packEventBasics(to);

    userAddress.pack(to);
    packCoinsInt(to, amount);
}

WithdrawEvent::WithdrawEvent(Netpack::Consumer* from)
    : Event(from), userAddress(zeroAddress)
{
    userAddress = Address(from);
    amount = consumeCoinsInt(from);
}

DepositEvent::DepositEvent(Address userAddress, coinsInt amount)
    : Event(),
      userAddress(userAddress), amount(amount)
      {}

void DepositEvent::execute(Game* game)
{
    uint8_t playerId = game->getPlayerId_createIfNone(userAddress);

    game->players[playerId].credit.createMoreByFiat(amount);
}

void DepositEvent::pack(Netpack::Builder* to)
{
    packEventBasics(to);

    userAddress.pack(to);
    packCoinsInt(to, amount);
}

DepositEvent::DepositEvent(Netpack::Consumer* from)
    : Event(from), userAddress(zeroAddress)
{
    userAddress = Address(from);
    amount = consumeCoinsInt(from);
}

uint8_t HoneypotAddedEvent::typechar()
{
    return EVENT_HONEYPOT_CHAR;
}

void HoneypotAddedEvent::execute(Game *game)
{
    boost::shared_ptr<GoldPile> gp(new GoldPile(vector2fp::zero));
    gp->gold.createMoreByFiat(honeypotAmount);
    game->registerNewEntityIgnoringConstraints(gp);
}

HoneypotAddedEvent::HoneypotAddedEvent(coinsInt honeypotAmount)
    : Event(), honeypotAmount(honeypotAmount)
    {}
void HoneypotAddedEvent::pack(Netpack::Builder* to)
{
    packEventBasics(to);
    packCoinsInt(to, honeypotAmount);
}
HoneypotAddedEvent::HoneypotAddedEvent(Netpack::Consumer *from)
    : Event(from)
{
    honeypotAmount = consumeCoinsInt(from);
}


uint8_t ResetBeaconsEvent::typechar()
{
    return EVENT_RESETBEACONS_CHAR;
}

void ResetBeaconsEvent::execute(Game *game)
{
    for (unsigned int i=0; i<game->players.size(); i++)
    {
        game->players[i].beaconAvailable = true;
    }
}

ResetBeaconsEvent::ResetBeaconsEvent()
    : Event()
    {}
void ResetBeaconsEvent::pack(Netpack::Builder* to)
{
    packEventBasics(to);
}
ResetBeaconsEvent::ResetBeaconsEvent(Netpack::Consumer *from)
    : Event(from)
{}
#include <iostream>
#include <boost/shared_ptr.hpp>
#include "events.h"
#include "common.h"
#include "netpack.h"
#include "config.h"

using namespace std;

boost::shared_ptr<Event> consumeEvent(Netpack::Consumer* from)
{
    uint8_t typechar = consumeTypechar(from);

    switch (typechar)
    {
    case NULL_TYPECHAR:
        return boost::shared_ptr<Event>();
    case EVENT_BALANCEUPDATE_CHAR:
        return boost::shared_ptr<Event>(new BalanceUpdateEvent(from));
    case EVENT_HONEYPOT_CHAR:
        return boost::shared_ptr<Event>(new HoneypotAddedEvent(from));
    }
    throw runtime_error("Trying to unpack an unrecognized event");
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

uint8_t BalanceUpdateEvent::typechar()
{
    return EVENT_BALANCEUPDATE_CHAR;
}
void BalanceUpdateEvent::execute(Game *game)
{
    if (isDeposit)
    {
        optional<uint8_t> maybePlayerId = game->playerAddressToMaybeId(userAddress);

        uint8_t playerId;
        if (maybePlayerId)
        {
            playerId = *maybePlayerId;
        }
        else
        {
            // if no user for this address, create one
            game->players.push_back(Player(userAddress));
            playerId = game->players.size() - 1;
        }

        game->players[playerId].credit.createMoreByFiat(amount);
    }
    else
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
}

BalanceUpdateEvent::BalanceUpdateEvent(Address userAddress, coinsInt amount, bool isDeposit)
    : Event(),
      userAddress(userAddress), amount(amount), isDeposit(isDeposit) {}
void BalanceUpdateEvent::pack(Netpack::Builder* to)
{
    packEventBasics(to);

    userAddress.pack(to);
    packCoinsInt(to, amount); 
    to->packBool(isDeposit);
}
BalanceUpdateEvent::BalanceUpdateEvent(Netpack::Consumer* from)
    : Event(from), userAddress(zeroAddress)
{
    userAddress = Address(from);
    // cout << "userAddress after unpack: " << userAddress << endl;
    amount = consumeCoinsInt(from);
    isDeposit = from->consumeBool();
}

uint8_t HoneypotAddedEvent::typechar()
{
    return EVENT_HONEYPOT_CHAR;
}

void HoneypotAddedEvent::execute(Game *game)
{
    boost::shared_ptr<GoldPile> gp(new GoldPile(vector2fp::zero));
    gp->gold.createMoreByFiat(honeypotAmount);
    game->registerNewEntityIgnoringCollision(gp);
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
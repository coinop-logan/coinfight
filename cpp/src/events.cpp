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

    // split it three ways: wallet, honeypot, profit.
    // fixed32 can't handle ints as big as amount,
    // so we use int math and careful order of operations to keep things deterministic.

    coinsInt toWallet = amount * DEPOSIT_TO_WALLET_PERCENT / 100;
    coinsInt toHoneypot = amount * DEPOSIT_TO_HONEYPOT_PERCENT / 100;
    coinsInt toProfit = amount - (toWallet + toHoneypot);

    game->players[playerId].credit.createMoreByFiat(toWallet);
    game->throwIntoHoneypotByFiat(toHoneypot);
    game->tallyProfit(toProfit);
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

// void GameEndEvent::execute(Game *game)
// {
//     // refund all units gold to owners' wallets
//     // add GPs to game profit

//     for (unsigned int i=0; i<game->entities.size(); i++)
//     {
//         if (auto goldpile = boost::dynamic_pointer_cast<GoldPile, Entity>(game->entities[i]))
//         {
//             game->tallyProfit(goldpile->gold.getInt());
//             goldpile->gold.destroySomeByFiat(goldpile->gold.getInt());
//         }
//         else if (auto unit = boost::dynamic_pointer_cast<Unit, Entity>(game->entities[i]))
//         {
//             auto droppableCoins = unit->getDroppableCoins();
//             for (unsigned int j=0; j<droppableCoins.size(); i++)
//             {
//                 bool success = droppableCoins[j]->tryTransfer(droppableCoins[j]->getInt(), &game->players[unit->ownerId].credit);
//                 if (!success)
//                 {
//                     cout << "DIRE WARNING: Could not transfer a unit's droppable coins into the user's wallet!" << endl;
//                 }
//             }
//         }
//     }

//     // units with 0 gold will just die. But more immediately,
//     // the server will see this event, send out withdrawals, and quit.
// }

// GameEndEvent::GameEndEvent()
//     : Event()
//     {}
// void GameEndEvent::pack(Netpack::Builder* to)
// {
//     packEventBasics(to);
// }
// GameEndEvent::GameEndEvent(Netpack::Consumer *from)
//     : Event(from)
// {}
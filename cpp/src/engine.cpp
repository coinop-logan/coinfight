#include <iostream>
#include <boost/shared_ptr.hpp>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <vector>
#include <string>
#include "myvectors.h"
#include "config.h"
#include "vchpack.h"
#include "engine.h"
#include "coins.h"
#include "events.h"

using namespace std;

sf::Color playerAddressToColor(string address)
{
    int vals[3];
    for (uint i=0; i<3; i++)
    {
        string charStr = address.substr(2 + i, 1);
        unsigned int intVal = std::stoul(charStr, nullptr, 16);
        vals[i] = 55 + ((intVal / 15.0) * 200);
    }
    return sf::Color(vals[0], vals[1], vals[2]);
}

EntityRef Game::getNextEntityRef()
{
    return entities.size() + 1;
}

void Player::pack(vch *dest)
{
    packStringToVch(dest, address);
    credit.pack(dest);
}
void Player::unpackAndMoveIter(vchIter *iter)
{
    *iter = unpackStringFromIter(*iter, 50, &address);
    credit = Coins(iter);
}

Player::Player(string address)
    : address(address), credit(), beaconAvailable(true) {}

Player::Player(vchIter *iter)
{
    unpackAndMoveIter(iter);
}

int Game::playerAddressToIdOrNegativeOne(string address)
{
    for (uint i=0; i<players.size(); i++)
    {
        if (players[i].address == address)
        {
            return i;
        }
    }
    return -1;
}
string Game::playerIdToAddress(uint playerId)
{
    return players[playerId].address;
}
bool Game::getPlayerBeaconAvailable(uint playerId)
{
    return players[playerId].beaconAvailable;
}
void Game::setPlayerBeaconAvailable(uint playerId, bool flag)
{
    players[playerId].beaconAvailable = flag;
}

void Game::killAndReplaceEntity(EntityRef ref, boost::shared_ptr<Entity> newEntity)
{
    entities[ref-1]->die();
    entities[ref-1] = newEntity;
}

void Game::pack(vch *dest)
{
    packToVch(dest, "C", (unsigned char)(state));
    packToVch(dest, "Q", frame);

    packToVch(dest, "C", (unsigned char)(players.size()));
    for (uint i=0; i < players.size(); i++)
    {
        players[i].pack(dest);
    }

    packToVch(dest, "H", (EntityRef)(entities.size()));
    for (EntityRef i = 0; i < entities.size(); i++)
    {
        unsigned char typechar = getMaybeNullEntityTypechar(entities[i]);

        packTypechar(dest, typechar);

        if (typechar != NULL_TYPECHAR)
        {
            entities[i]->pack(dest);
        }
    }
}
void Game::unpackAndMoveIter(vchIter *iter)
{
    unsigned char enumInt;
    *iter = unpackFromIter(*iter, "C", &enumInt);
    state = static_cast<State>(enumInt);

    *iter = unpackFromIter(*iter, "Q", &frame);
    
    uint8_t playersSize;
    *iter = unpackFromIter(*iter, "C", &playersSize);
    players.clear();

    for (int i = 0; i < playersSize; i++)
    {
        players.push_back(Player(iter));
    }

    uint16_t entitiesSize;
    *iter = unpackFromIter(*iter, "H", &entitiesSize);
    entities.clear();

    for (int i = 0; i < entitiesSize; i++)
    {
        unsigned char typechar;
        *iter = unpackTypecharFromIter(*iter, &typechar);

        entities.push_back(unpackFullEntityAndMoveIter(iter, typechar, this, getNextEntityRef()));
    }
}

Game::Game() : state(Pregame), frame(0) {}
Game::Game(vchIter *iter)
{
    unpackAndMoveIter(iter);
}

void Game::startMatch()
{
    // float spawnCircleCircumference = SPACE_BETWEEN_SPAWNS * players.size();
    // float spawnCircleRadius = spawnCircleCircumference / 2;
    // for (uint i=0; i<players.size(); i++)
    // {
    //     float positionAlongCircumference = ((float)i)/players.size();
    //     float spawnAngle = positionAlongCircumference * 2 * M_PI;
    //     vector2f spawnPos = composeVector2f(spawnAngle, spawnCircleRadius);

    //     // if you're going to change this, you should change neededCostPerPlayer above too!
    //     boost::shared_ptr<Unit> beaconUnit(new Beacon(this, getNextEntityRef(), i, spawnPos));
    //     entities.push_back(beaconUnit);
    // }

    state = Active;
}

void Game::startMatchOrPrintError()
{
    coinsInt neededCreditPerPlayer = BEACON_COST;
    for (uint i=0; i<players.size(); i++)
    {
        if (players[i].credit.getInt() < neededCreditPerPlayer)
        {
            cout << "Player " << players[i].address << " doesn't have enough credit! They currently have " << players[i].credit.getInt() << " but they need " << neededCreditPerPlayer << "." << endl;
            return;            
        }
    }

    startMatch();
    
    cout << "game starting!" << endl;
}

void Game::reassignEntityGamePointers()
{
    for (EntityRef i = 0; i < entities.size(); i++)
    {
        if (entities[i])
            entities[i]->game = this;
    }
}

void Game::iterate()
{
    switch (state)
    {
        case Pregame:
            break;
        case Active:
            // iterate all units
            for (uint i=0; i<entities.size(); i++)
            {
                if (entities[i])
                {
                    if (auto unit = boost::dynamic_pointer_cast<Unit, Entity>(entities[i]))
                    {
                        if (unit->isActive())
                            unit->go();
                    }
                    else
                        entities[i]->go();
                }
            }

            // clean up units that are ded
            for (uint i=0; i<entities.size(); i++)
            {
                if (entities[i] && entities[i]->dead)
                {
                    vector<Coins*> coins = entities[i]->getDroppableCoins();
                    for (uint j=0; j<coins.size(); j++)
                    {
                        if (coins[j]->getInt() > 0)
                        {
                            boost::shared_ptr<GoldPile> goldPile(new GoldPile(this, getNextEntityRef(), entities[i]->pos));
                            entities.push_back(goldPile);
                            coins[j]->transferUpTo(coins[j]->getInt(), &goldPile->gold);
                        }
                    }
                    entities[i].reset();
                }
            }

            // Build/debuild beacons and transform/kill them
            for (uint i=0; i<entities.size(); i++)
            {
                if (entities[i])
                {
                    if (auto beacon = boost::dynamic_pointer_cast<Beacon, Entity>(entities[i]))
                    {
                        switch (beacon->state)
                        {
                            case Beacon::Spawning:
                            {
                                beacon->build(BEACON_BUILD_RATE, &players[beacon->ownerId].credit);

                                if (beacon->isActive())
                                {
                                    boost::shared_ptr<Gateway> transformed(new Gateway(this, beacon->ref, beacon->ownerId, beacon->pos));
                                    transformed->completeBuildingInstantly(&beacon->goldInvested);
                                    entities[i] = transformed;
                                }
                            }
                            break;
                            case Beacon::Despawning:
                            {
                                beacon->unbuild(BEACON_BUILD_RATE, &players[beacon->ownerId].credit);

                                if (beacon->getBuilt() == 0)
                                {
                                    players[beacon->ownerId].beaconAvailable = true;
                                    beacon->die();
                                }
                            }
                            break;
                        }
                    }
                }
            }

            frame++;

            if (frame % 200 == 0)
            {
                for (uint i=0; i<players.size(); i++)
                {
                    cout << "player " << players[i].address << " has " << players[i].credit.getInt() << endl;
                }
            }
            break;
    }
}
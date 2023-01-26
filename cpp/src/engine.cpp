#include <iostream>
#include <boost/shared_ptr.hpp>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <vector>
#include <string>
#include "myvectors.h"
#include "config.h"
#include "netpack.h"
#include "engine.h"
#include "coins.h"
#include "events.h"
#include "collision.h"
#include "fpm/math.hpp"

using namespace std;

bool Game::registerNewEntityIfInMapAndNoCollision(boost::shared_ptr<Entity> newEntity)
{
    if (this->unitsCollidingWithCircle(newEntity->getPos(), newEntity->getRadius()).size() > 0)
    {
        return false;
    }
    
    return registerNewEntityIfInMapIgnoringCollision(newEntity);
}
bool Game::registerNewEntityIfInMapIgnoringCollision(boost::shared_ptr<Entity> newEntity)
{
    if (newEntity && newEntity->getPos().getRoughMagnitude() > mapRadius)
    {
        return false;
    }

    // if registering a null entity, just add the empty pointer to the list but don't do anything else
    if (!newEntity)
    {
        entities.push_back(newEntity);
        return true;
    }

    registerNewEntityIgnoringConstraints(newEntity);

    return true;
}

void Game::registerNewEntityIgnoringConstraints(boost::shared_ptr<Entity> newEntity)
{
    EntityRef ref = this->entities.size();

    optional<vector2i> maybeCell = searchGrid.registerEntityRefToCell(newEntity, ref);
    if (!maybeCell)
    {
        throw runtime_error("Entity can't be registered on the search grid - probably out of bounds.\n");
    }

    RegInfo regInfo(this, ref, *maybeCell);
    newEntity->maybeRegInfo = {regInfo};

    entities.push_back(newEntity);
}

fixed32 calculateMapRadius()
{
    return calculateMapRingRadius(NUM_MAP_RINGS);
}
fixed32 calculateMapRingRadius(int ringNum)
{
    return 10 * pow(fixed32(2), fixed32(ringNum + 1));
}

Player::Player(Address address)
    : address(address), credit(), beaconAvailable(true) {}
void Player::pack(Netpack::Builder* to)
{
    address.pack(to);
    to->packBool(beaconAvailable);
    credit.pack(to);
}
Player::Player(Netpack::Consumer* from)
    : address(from)
{
    beaconAvailable = from->consumeBool();
    credit = Coins(from);
}

SearchGridRect::SearchGridRect(vector2i start, vector2i end)
    : start(start), end(end) {}

bool SearchGrid::cellIsValid(vector2i cell)
{
    return (!
        (
            cell.x < 0 ||
            (unsigned int)(cell.x) >= getNumCellRows() ||
            cell.y < 0 ||
            (unsigned int)(cell.y) >= getNumCellRows()
        )
    );
}
void SearchGrid::registerEntityForCellOrThrow(vector2i cell, EntityRef entityRef)
{
    if (!cellIsValid(cell))
    {
        throw logic_error("Trying to SearchGrid::registerEntityForCellOrThrow, but cell is out of bounds\n");
    }

    auto insertResult = cells[cell.x][cell.y].insert(entityRef);
    if (!insertResult.second)
    {
        throw logic_error("Trying to SearchGrid::registerEntityForCellOrThrow, but it has already been registered here\n");
    }
}
void SearchGrid::deregisterEntityFromCellOrThrow(vector2i cell, EntityRef entityRef)
{
    if (!cellIsValid(cell))
    {
        throw logic_error("Trying to SearchGrid::deregisterEntityForCellOrThrow, but cell is out of bounds\n");
    }
    
    uint8_t numErased = cells[cell.x][cell.y].erase(entityRef);
    if (numErased == 0)
    {
        throw logic_error("Trying to SearchGrid::registerEntityForCellOrThrow, but it wasn't registered here\n");
    }
}

vector2fp SearchGrid::gamePosToCellSpace(vector2fp gamePos)
{
    return (gamePos - originInGameSpace) / SEARCH_GRID_CELL_WIDTH;
}

vector2i SearchGrid::gamePosToCellConstrained(vector2fp gamePos)
{
    vector2fp cellSpacePos = gamePosToCellSpace(gamePos);
    cellSpacePos.x = min((fixed32)getNumCellRows()-1, max(fixed32(0), cellSpacePos.x));
    cellSpacePos.y = min((fixed32)getNumCellRows()-1, max(fixed32(0), cellSpacePos.y));
    return cellSpacePos;
}

SearchGrid::SearchGrid(fixed32 mapRadius)
{
    originInGameSpace = vector2fp(-fixed32(mapRadius + 10), -fixed32(mapRadius + 10));

    vector2fp rightmostPlausibleGamePoint(mapRadius + fixed32(10), fixed32(0));
    vector2fp rightmostPlausibleCellPoint = gamePosToCellSpace(rightmostPlausibleGamePoint);
    
    unsigned int rowsNeeded = rightmostPlausibleCellPoint.floored().x + 1;

    cells.resize(rowsNeeded);
    for (unsigned int i=0; i<rowsNeeded; i++)
    {
        cells[i].resize(rowsNeeded);
    }
}
unsigned int SearchGrid::getNumCellRows()
{
    return cells.size();
}
vector2fp SearchGrid::getOriginInGameSpace()
{
    return originInGameSpace;
}
set<EntityRef> SearchGrid::getCell(vector2i cell)
{
    return cells[cell.x][cell.y];
}
optional<vector2i> SearchGrid::gamePosToCell(vector2fp gamePos)
{
    vector2fl gamePosFl(gamePos);
    vector2fp gamePosInSearchGridSpace = gamePosToCellSpace(gamePos);
    vector2fl test(gamePosInSearchGridSpace);

    if (gamePosInSearchGridSpace.x < fixed32(0) ||
        gamePosInSearchGridSpace.x >= fixed32(getNumCellRows()) ||
        gamePosInSearchGridSpace.y < fixed32(0) ||
        gamePosInSearchGridSpace.y >= fixed32(getNumCellRows()))
    {
        return {};
    }
    else
    {
        return {gamePosInSearchGridSpace.floored()};
    }
}
optional<vector2i> SearchGrid::registerEntityRefToCell(boost::shared_ptr<Entity> entity, EntityRef ref)
{
    // check if it is in the search grid at all, and verify it has not yet been registered
    if (auto cell = gamePosToCell(entity->getPos()))
    {
        if (!entity->maybeRegInfo)
        {
            registerEntityForCellOrThrow(*cell, ref);

            return cell;
        }
    }

    return {};
}
optional<vector2i> SearchGrid::updateEntityCellRelation(Entity* entity)
{
    // check if it's in the search grid
    if (auto newCell = gamePosToCell(entity->getPos()))
    {
        // will throw if it hasn't already been registered
        auto oldCell = entity->getSearchGridCellOrThrow();

        if (*newCell == oldCell)
            return {oldCell};

        deregisterEntityFromCellOrThrow(oldCell, entity->getRefOrThrow());
        registerEntityForCellOrThrow(*newCell, entity->getRefOrThrow());

        entity->maybeRegInfo->cell = *newCell;

        return newCell;
    }

    return {};
}
SearchGridRect SearchGrid::gridRectAroundGamePos(vector2fp gamePos, fixed32 radius)
{
    vector2fp startGamePos = gamePos - vector2fp(radius, radius);
    vector2fp endGamePos = gamePos + vector2fp(radius, radius);

    vector2i startCell = gamePosToCellConstrained(startGamePos);
    vector2i endCell = gamePosToCellConstrained(endGamePos);
    
    return SearchGridRect(startCell, endCell);
}
vector<EntityRef> SearchGrid::entitiesInGridRect(SearchGridRect rect)
{
    vector<EntityRef> entities;
    for (int i=rect.start.x; i<=rect.end.x; i++)
    {
        for (int j=rect.start.y; j<=rect.end.y; j++)
        {
            copy(cells[i][j].begin(), cells[i][j].end(), back_inserter(entities));
        }
    }
    return entities;
}
vector<EntityRef> SearchGrid::nearbyEntitiesSloppyIncludingEmpty(vector2fp gamePos, fixed32 radius)
{
    return entitiesInGridRect(gridRectAroundGamePos(gamePos, radius));
}

optional<uint8_t> Game::playerAddressToMaybeId(Address address)
{
    for (uint8_t i=0; i<players.size(); i++)
    {
        if (players[i].address == address)
        {
            return {i};
        }
    }
    return {};
}
uint8_t Game::getPlayerId_createIfNone(Address userAddress)
{
    optional<uint8_t> maybePlayerId = playerAddressToMaybeId(userAddress);

    uint8_t playerId;
    if (maybePlayerId)
    {
        playerId = *maybePlayerId;
    }
    else
    {
        // if no user for this address, create one
        players.push_back(Player(userAddress));
        playerId = players.size() - 1;
    }

    return playerId;
}
Address Game::playerIdToAddress(uint8_t playerId)
{
    return players[playerId].address;
}
bool Game::getPlayerBeaconAvailable(uint8_t playerId)
{
    return players[playerId].beaconAvailable;
}
void Game::setPlayerBeaconAvailable(uint8_t playerId, bool flag)
{
    players[playerId].beaconAvailable = flag;
}
vector<boost::shared_ptr<Entity>> Game::entitiesWithinCircle(vector2fp fromPos, fixed32 radius)
{
    uint32_t radiusFloorSquared = floorSquareFixed(radius);

    auto nearbyEntityRefs = searchGrid.nearbyEntitiesSloppyIncludingEmpty(fromPos, radius);

    vector<boost::shared_ptr<Entity>> entitiesToReturn;
    for (unsigned int i=0; i<nearbyEntityRefs.size(); i++)
    {
        if (auto entity = this->entities[nearbyEntityRefs[i]]) // checks for non-empty pointer
        {
            if ((fromPos - entity->getPos()).getFloorMagnitudeSquared() <= radiusFloorSquared)
            {
                entitiesToReturn.push_back(entity);
            }
        }
    }

    return entitiesToReturn;
}
vector<boost::shared_ptr<Entity>> Game::entitiesWithinSquare(vector2fp centerPos, fixed32 halfWidth)
{
    auto nearbyEntityRefs = searchGrid.nearbyEntitiesSloppyIncludingEmpty(centerPos, halfWidth);

    vector<boost::shared_ptr<Entity>> entitiesToReturn;
    for (unsigned int i=0; i<nearbyEntityRefs.size(); i++)
    {
        if (auto entity = this->entities[nearbyEntityRefs[i]]) // checks for non-empty pointer
        {
            vector2fp entityPos = entity->getPos();
            if (abs(centerPos.x - entityPos.x) <= halfWidth && abs(centerPos.y - entityPos.y) <= halfWidth)
            {
                entitiesToReturn.push_back(entity);
            }
        }
    }

    return entitiesToReturn;
}
vector<boost::shared_ptr<Entity>> Game::entitiesWithinRect(vector2fp corner1, vector2fp corner2)
{
    fixed32 minX = min(corner1.x, corner2.x);
    fixed32 maxX = max(corner1.x, corner2.x);
    fixed32 minY = min(corner1.y, corner2.y);
    fixed32 maxY = max(corner1.y, corner2.y);

    vector2fp centerPos = (corner1 + corner2) / 2;
    fixed32 halfWidth = max(maxX - minX, maxY - minY);

    auto nearbyEntityRefs = searchGrid.nearbyEntitiesSloppyIncludingEmpty(centerPos, halfWidth);

    vector<boost::shared_ptr<Entity>> entitiesToReturn;
    for (unsigned int i=0; i<nearbyEntityRefs.size(); i++)
    {
        if (auto entity = this->entities[nearbyEntityRefs[i]]) // checks for non-empty pointer
        {
            vector2fp entityPos = entity->getPos();
            if (entityPos.x >= minX && entityPos.x <= maxX && entityPos.y >= minY && entityPos.y <= maxY)
            {
                entitiesToReturn.push_back(entity);
            }
        }
    }

    return entitiesToReturn;
}
vector<boost::shared_ptr<Unit>> Game::unitsCollidingWithCircle(vector2fp centerPos, fixed32 radius)
{
    auto nearbyEntityRefs = searchGrid.nearbyEntitiesSloppyIncludingEmpty(centerPos, radius + MAX_UNIT_RADIUS);

    vector<boost::shared_ptr<Unit>> unitsToReturn;
    for (unsigned int i=0; i<nearbyEntityRefs.size(); i++)
    {
        if (auto unit = boost::dynamic_pointer_cast<Unit, Entity>(this->entities[nearbyEntityRefs[i]]))
        {
            fixed32 combinedRadius = radius + unit->getRadius();
            uint32_t combinedRadiusFloorSquared = floorSquareFixed(combinedRadius);
            if ((centerPos - unit->getPos()).getFloorMagnitudeSquared() < combinedRadiusFloorSquared)
            {
                unitsToReturn.push_back(unit);
            }
        }
    }

    return unitsToReturn;
}


Game::Game()
    : frame(0), searchGrid(calculateMapRadius())
{
    mapRadius = calculateMapRadius();
}

void Game::pack(Netpack::Builder* to)
{
    to->packUint64_t(frame);
    to->packUint8_t((uint8_t)players.size());
    for (unsigned int i=0; i < players.size(); i++)
    {
        players[i].pack(to);
    }

    to->packUint16_t((uint16_t)entities.size());
    for (EntityRef i = 0; i < entities.size(); i++)
    {
        uint8_t typechar = getMaybeNullEntityTypechar(entities[i]);

        packTypechar(to, typechar);

        if (typechar != NULL_TYPECHAR)
        {
            entities[i]->pack(to);
        }
    }
}
Game::Game(Netpack::Consumer* from)
    : searchGrid(calculateMapRadius())
    , mapRadius(calculateMapRadius())
{
    frame = from->consumeUint64_t();
    uint8_t playersSize = from->consumeUint8_t();
    
    players.clear();
    for (unsigned int i = 0; i < playersSize; i++)
    {
        players.push_back(Player(from));
    }

    uint16_t entitiesSize = from->consumeUint16_t();

    entities.clear();
    for (unsigned int i = 0; i < entitiesSize; i++)
    {
        auto entity = consumeEntity(from);
        if (!entity)
        {
            throw runtime_error("Trying to unpack unrecognized entity");
        }
        bool registerSuccess = registerNewEntityIfInMapIgnoringCollision(entity);
        if (!registerSuccess)
        {
            throw "Error registering entity as encoded: it's beyond map bounds.";
        }
    }
}
void Game::reassignEntityGamePointers()
{
    for (EntityRef i = 0; i < entities.size(); i++)
    {
        if (entities[i])
            entities[i]->updateGamePointerOrThrow(this);
    }
}

void Game::iterate()
{
    // iterate all units
    for (unsigned int i=0; i<entities.size(); i++)
    {
        if (entities[i])
        {
            if (auto unit = boost::dynamic_pointer_cast<Unit, Entity>(entities[i]))
            {
                if (unit->isActive() || unit->typechar() == BEACON_TYPECHAR)
                    unit->iterate();
            }
            else
                entities[i]->iterate();
        }
    }

    // for MobileUnits, correct velocities and move 
    for (unsigned int i=0; i<entities.size(); i++)
    {
        // only for MobileUnits
        if (auto mobileUnit = boost::dynamic_pointer_cast<MobileUnit, Entity>(entities[i]))
        {
            if (mobileUnit->isActive())
            {
                auto nearbyEntities = this->entitiesWithinSquare(mobileUnit->getPos(), COLLISION_CORRECTION_BROADPHASE_FILTERBOX_HALFWIDTH);

                // filter for Units (this ignores GoldPiles)
                auto nearbyUnits = filterForType<Unit, Entity>(nearbyEntities);

                // quickly remove the inevitable self-reference
                for (unsigned int j=0; j<nearbyUnits.size(); j++)
                {
                    // don't compare to self
                    if (mobileUnit->getRefOrThrow() == nearbyUnits[j]->getRefOrThrow())
                    {
                        nearbyUnits.erase(nearbyUnits.begin() + j);
                        break;
                    }
                }

                vector2fp velocity = calcNewVelocityToAvoidCollisions(mobileUnit, nearbyUnits, fixed32(40), fixed32(1));
                // cout << ((velocity - mobileUnit->getDesiredVelocity()).getMagnitudeSquared() < EPSILON) << endl;

                mobileUnit->moveWithVelocityAndUpdateCell(velocity);
            }
        }
    }

    // clean up units that are ded
    // also take note of GWs, Primes, and zeroed goldpiles for purposes of the next loop
    vector<EntityRef> goldpilesToMaybeDelete;
    vector<boost::shared_ptr<Unit>> gatewaysAndPrimes;

    for (unsigned int i=0; i<entities.size(); i++)
    {
        if (entities[i])
        {
            if (entities[i]->dead)
            {
                // create new GoldPile to hold all droppable coins
                boost::shared_ptr<GoldPile> goldPile(new GoldPile(entities[i]->getPos()));

                vector<Coins*> droppableCoins = entities[i]->getDroppableCoins();
                for (unsigned int j=0; j<droppableCoins.size(); j++)
                {
                    if (droppableCoins[j]->getInt() > 0)
                    {
                        droppableCoins[j]->transferUpTo(droppableCoins[j]->getInt(), &goldPile->gold);
                    }
                }

                // but only add it if there was more than 0 gold added
                if (goldPile->gold.getInt() > 0)
                {
                    // We call the 'ignoreConstraints' verison, knowing that at the end of Game::iterate, map bounds are directly enforced
                    registerNewEntityIgnoringConstraints(goldPile);
                }
                entities[i].reset();
            }
            else
            {
                if (auto goldpile = boost::dynamic_pointer_cast<GoldPile, Entity>(entities[i]))
                {
                    if (goldpile->gold.getInt() == 0)
                    {
                        goldpilesToMaybeDelete.push_back(goldpile->getRefOrThrow());
                    }
                }
                else if (auto gateway = boost::dynamic_pointer_cast<Gateway, Entity>(entities[i]))
                {
                    gatewaysAndPrimes.push_back(gateway);
                }
                else if (auto prime = boost::dynamic_pointer_cast<Prime, Entity>(entities[i]))
                {
                    gatewaysAndPrimes.push_back(prime);
                }
            }
        }
    }

    // Now we just remove all goldpiles with 0 gold, if they are not referenced by a Prime or GW
    for (unsigned int i=0; i<goldpilesToMaybeDelete.size(); i++)
    {
        auto goldpileRef = goldpilesToMaybeDelete[i];

        bool referenceFound;
        for (unsigned int j=0; j<gatewaysAndPrimes.size(); j++)
        {
            if (auto gatewayOrPrime = gatewaysAndPrimes[j])
            {
                if (auto prime = boost::dynamic_pointer_cast<Prime, Unit>(gatewayOrPrime))
                {
                    referenceFound =
                    (    (prime->fundsDest && *prime->fundsDest == goldpileRef)
                      || (prime->fundsSource && *prime->fundsSource == goldpileRef)
                      || (prime->isInBuildTargetQueue(goldpileRef))
                      || (prime->isInScavengeTargetQueue(Target(goldpileRef)))
                    );
                }
                else if (auto gateway = boost::dynamic_pointer_cast<Gateway, Unit>(gatewayOrPrime))
                {
                    referenceFound =
                    (    (gateway->isInBuildTargetQueue(goldpileRef))
                      || (gateway->isInScuttleTargetQueue(goldpileRef))
                    );
                }
            }

            if (referenceFound)
                break;
        }

        if (!referenceFound)
        {
            // no reference found, so we delete it from the entities list.
            entities[goldpileRef].reset();
        }
    }

    // hard-constrain units to mapRadius
    for (unsigned int i=0; i<entities.size(); i++)
    {
        if (auto entity = entities[i])
        {
            fixed32 roughMagnitude = entity->getPos().getRoughMagnitude();
            if (roughMagnitude > mapRadius)
            {
                fixed32 correctionRatio = mapRadius / roughMagnitude;
                debugOutputVector("   pos",entity->getPos());
                vector2fp newPos(entity->getPos().x * correctionRatio, entity->getPos().y * correctionRatio);
                debugOutputVector("newpos",newPos);
                entity->setPosAndUpdateCell(newPos);
            }
        }
    }

    frame++;

    if (frame % 200 == 0)
    {
        for (unsigned int i=0; i<players.size(); i++)
        {
            // cout << "player " << players[i].address << " has " << players[i].credit.getInt() << endl;
        }
    }
}

bool gameStatesAreIdentical_triggerDebugIfNot(Game* game1, Game* game2)
{
    if (game1->frame != game2->frame
     || game1->entities.size() != game2->entities.size()
     || game1->players.size() != game2->players.size()
       )
    {
        triggerDebug();
        return false;
    }
    
    for (unsigned int i=0; i<game1->players.size(); i++)
    {
        if (game1->players[i].address != game2->players[i].address)
        {
            triggerDebug();
            return false;
        }
        if (game1->players[i].beaconAvailable != game2->players[i].beaconAvailable)
        {
            triggerDebug();
            return false;
        }
        if (game1->players[i].credit.getInt() != game2->players[i].credit.getInt())
        {
            triggerDebug();
            return false;
        }
    }

    for (unsigned int i=0; i<game1->entities.size(); i++)
    {
        if (!entitiesAreIdentical_triggerDebugIfNot(game1->entities[i], game2->entities[i]))
        {
            return false;
        }
    }

    return true;
}

void triggerDebug()
{
    int a;
    a = 3; // meant to be tagged in an IDE with a breakpoint.
    a ++;
}
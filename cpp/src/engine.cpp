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
    for (unsigned int i=0; i<3; i++)
    {
        string charStr = address.substr(2 + i, 1);
        unsigned int intVal = std::stoul(charStr, nullptr, 16);
        vals[i] = 55 + ((intVal / 15.0) * 200);
    }
    return sf::Color(vals[0], vals[1], vals[2]);
}

void Game::registerNewEntity(boost::shared_ptr<Entity> newEntity)
{
    // * register the entity on the search grid
    // * provide pointer to Game
    // * provide EntityRef ref
    // * add to Game::entities

    EntityRef ref = this->entities.size();
    optional<vector2i> maybeCell = searchGrid.registerEntityRefToCell(newEntity, ref);

    if (auto cell = maybeCell)
    {
        RegInfo regInfo(this, ref, *cell);
        newEntity->maybeRegInfo = {regInfo};
        entities.push_back(newEntity);
    }
    else
    {
        throw runtime_error("Entity can't be registered on the search grid - probably out of bounds.\n");
    }
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

SearchGridRect::SearchGridRect(vector2i start, vector2i end)
    : start(start), end(end) {}

bool SearchGrid::cellIsValid(vector2i cell)
{
    return (!
        (
            cell.x < 0 ||
            cell.x >= SEARCH_GRID_NUM_ROWS ||
            cell.y < 0 ||
            cell.y >= SEARCH_GRID_NUM_ROWS
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
    
    int numErased = cells[cell.x][cell.y].erase(entityRef);
    if (numErased == 0)
    {
        throw logic_error("Trying to SearchGrid::registerEntityForCellOrThrow, but it wasn't registered here\n");
    }
}

vector2f SearchGrid::gamePosToCellSpace(vector2f gamePos)
{
    // since map center is (0,0), search grid should be centered on (0,0)
    float halfSearchGridWidth = SEARCH_GRID_TOTAL_WIDTH / 2.0;
    vector2f searchGridOriginInGameSpace(-halfSearchGridWidth, -halfSearchGridWidth);
    return (gamePos - searchGridOriginInGameSpace) / SEARCH_GRID_CELL_WIDTH;
}

vector2i SearchGrid::gamePosToCellConstrained(vector2f gamePos)
{
    vector2f cellSpacePos = gamePosToCellSpace(gamePos);
    cellSpacePos.x = min((float)SEARCH_GRID_NUM_ROWS-1, max(0.0f, cellSpacePos.x));
    cellSpacePos.y = min((float)SEARCH_GRID_NUM_ROWS-1, max(0.0f, cellSpacePos.y));
    return cellSpacePos;
}

SearchGrid::SearchGrid() {}
optional<vector2i> SearchGrid::gamePosToCell(vector2f gamePos)
{
    vector2f gamePosInSearchGridSpace = gamePosToCellSpace(gamePos);

    if (gamePosInSearchGridSpace.x < 0 ||
        gamePosInSearchGridSpace.x >= SEARCH_GRID_NUM_ROWS ||
        gamePosInSearchGridSpace.y < 0 ||
        gamePosInSearchGridSpace.y >= SEARCH_GRID_NUM_ROWS)
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
    if (auto cell = gamePosToCell(entity->pos))
    {
        if (!entity->maybeRegInfo)
        {
            registerEntityForCellOrThrow(*cell, ref);

            return cell;
        }
    }

    return {};
}
optional<vector2i> SearchGrid::updateEntityCellRelation(boost::shared_ptr<Entity> entity)
{
    // check if it's in the search grid
    if (auto newCell = gamePosToCell(entity->pos))
    {
        // will throw if it hasn't already been registered
        auto oldCell = entity->getSearchGridCellOrThrow();

        if (newCell = oldCell)
            return {oldCell};

        deregisterEntityFromCellOrThrow(oldCell, entity->getRefOrThrow());
        registerEntityForCellOrThrow(*newCell, entity->getRefOrThrow());

        entity->maybeRegInfo->cell = *newCell;

        return newCell;
    }

    return {};
}
SearchGridRect SearchGrid::gridRectAroundGamePos(vector2f gamePos, float radius)
{
    vector2f startGamePos = gamePos - vector2f(radius, radius);
    vector2f endGamePos = gamePos + vector2f(radius, radius);

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
vector<EntityRef> SearchGrid::entitiesNearGamePosSloppy(vector2f gamePos, float radius)
{
    return entitiesInGridRect(gridRectAroundGamePos(gamePos, radius));
}

int Game::playerAddressToIdOrNegativeOne(string address)
{
    for (unsigned int i=0; i<players.size(); i++)
    {
        if (players[i].address == address)
        {
            return i;
        }
    }
    return -1;
}
string Game::playerIdToAddress(unsigned int playerId)
{
    return players[playerId].address;
}
bool Game::getPlayerBeaconAvailable(unsigned int playerId)
{
    return players[playerId].beaconAvailable;
}
void Game::setPlayerBeaconAvailable(unsigned int playerId, bool flag)
{
    players[playerId].beaconAvailable = flag;
}

// void Game::killAndReplaceEntity(EntityRef ref, boost::shared_ptr<Entity> newEntity)
// {
//     entities[ref]->die();
//     entities[ref] = newEntity;
    
// }

void Game::pack(vch *dest)
{
    packToVch(dest, "C", (unsigned char)(state));
    packToVch(dest, "Q", frame);

    packToVch(dest, "C", (unsigned char)(players.size()));
    for (unsigned int i=0; i < players.size(); i++)
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

        registerNewEntity(unpackFullEntityAndMoveIter(iter, typechar));
    }
}

Game::Game() : state(Active), frame(0) {}
Game::Game(vchIter *iter)
{
    unpackAndMoveIter(iter);
}

// void Game::reassignEntityGamePointers()
// {
//     for (EntityRef i = 0; i < entities.size(); i++)
//     {
//         if (entities[i])
//             entities[i]->game = this;
//     }
// }

void Game::iterate()
{
    switch (state)
    {
        case Pregame:
            break;
        case Active:
            // iterate all units
            for (unsigned int i=0; i<entities.size(); i++)
            {
                if (entities[i])
                {
                    if (auto unit = boost::dynamic_pointer_cast<Unit, Entity>(entities[i]))
                    {
                        if (unit->isActive() || unit->typechar() == BEACON_TYPECHAR)
                            unit->go();
                    }
                    else
                        entities[i]->go();
                }
            }

            // clean up units that are ded
            for (unsigned int i=0; i<entities.size(); i++)
            {
                if (entities[i] && entities[i]->dead)
                {
                    // create new GoldPile to hold all droppable coins
                    boost::shared_ptr<GoldPile> goldPile(new GoldPile(entities[i]->pos));

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
                        registerNewEntity(goldPile);
                    }
                    entities[i].reset();
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
            break;
    }
}

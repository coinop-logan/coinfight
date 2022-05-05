#include <iostream>
#include <boost/shared_ptr.hpp>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <vector>
#include <string>
#include <pthread.h>
#include <set>
#include "coins.h"
#include "myvectors.h"
#include "netpack.h"
#include "common.h"
#include "events.h"
#include "entities.h"

#ifndef ENGINE_H
#define ENGINE_H

using namespace std;

const int SEARCH_GRID_NUM_ROWS = 200;
const int SEARCH_GRID_CELL_WIDTH = 200;
const int SEARCH_GRID_TOTAL_WIDTH = SEARCH_GRID_NUM_ROWS * SEARCH_GRID_CELL_WIDTH;

sf::Color playerAddressToColor(string address);

class Game;

struct Player
{
    string address;
    Coins credit;
    bool beaconAvailable;

    void pack(Netpack::Builder*);

    Player(string address);
    Player(Netpack::Consumer*);
};

struct SearchGridRect
{
    vector2i start, end;
    SearchGridRect(vector2i start, vector2i end);
};

class SearchGrid
{
    set<EntityRef> cells[SEARCH_GRID_NUM_ROWS][SEARCH_GRID_NUM_ROWS];
    bool cellIsValid(vector2i cell);
    void registerEntityForCellOrThrow(vector2i cell, EntityRef entityRef);
    void deregisterEntityFromCellOrThrow(vector2i cell, EntityRef entityRef);
    vector2fp gamePosToCellSpace(vector2fp gamePos);
    vector2i gamePosToCellConstrained(vector2fp gamePos);
public:
    SearchGrid();
    set<EntityRef> getCell(vector2i cell);
    optional<vector2i> gamePosToCell(vector2fp gamePos);
    optional<vector2i> registerEntityRefToCell(boost::shared_ptr<Entity> entity, EntityRef ref);
    optional<vector2i> updateEntityCellRelation(Entity* entity);
    SearchGridRect gridRectAroundGamePos(vector2fp gamePos, fixed32 radius);
    vector<EntityRef> entitiesInGridRect(SearchGridRect rect);
    vector<EntityRef> nearbyEntitiesSloppyIncludingEmpty(vector2fp gamePos, fixed32 radius);
};

class Game
{
public:
    uint64_t frame;
    vector<Player> players;
    vector<boost::shared_ptr<Entity>> entities;
    SearchGrid searchGrid;

    bool registerNewEntityIfNoCollision(boost::shared_ptr<Entity> newEntity);
    void registerNewEntityIgnoringCollision(boost::shared_ptr<Entity> newEntity);
    boost::shared_ptr<Entity> maybeEntityRefToPtrOrNull(EntityRef);

    int playerAddressToIdOrNegativeOne(string address);
    string playerIdToAddress(unsigned int playerId);
    bool getPlayerBeaconAvailable(unsigned int playerId);
    void setPlayerBeaconAvailable(unsigned int playerId, bool flag);

    vector<boost::shared_ptr<Entity>> entitiesWithinCircle(vector2fp centerPos, fixed32 radius);
    vector<boost::shared_ptr<Entity>> entitiesWithinSquare(vector2fp centerPos, fixed32 halfWidth);
    vector<boost::shared_ptr<Unit>> unitsCollidingWithCircle(vector2fp centerPos, fixed32 radius);

    void pack(Netpack::Builder*);

    Game();
    Game(Netpack::Consumer*);
    void reassignEntityGamePointers();

    void iterate();
};

vector<EntityRef> entityPtrsToRefsOrThrow(vector<boost::shared_ptr<Entity>>);
vector<EntityRef> entityPtrsToRefsOrThrow(vector<boost::shared_ptr<Unit>>);
boost::shared_ptr<Entity> maybeEntityRefToPtrOrNull(const Game&, optional<EntityRef>);

bool gameStatesAreIdentical_triggerDebugIfNot(Game* game1, Game* game2);
void triggerDebug();

#endif // ENGINE_H

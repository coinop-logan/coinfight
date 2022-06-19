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

class Address
{
    string s;
public:
    Address(string s);
    Address(Netpack::Consumer*);
    string getString() const;
    void pack(Netpack::Builder*);
    bool operator ==(const Address &other);
    bool operator !=(const Address &other);
};

sf::Color playerAddressToColor(Address address);

class Game;

const Address zeroAddress("0x0000000000000000000000000000000000000000");

struct Player
{
    Address address;
    Coins credit;
    bool beaconAvailable;

    void pack(Netpack::Builder*);

    Player(Address);
    Player(Netpack::Consumer*);
};

struct SearchGridRect
{
    vector2i start, end;
    SearchGridRect(vector2i start, vector2i end);
};

class SearchGrid
{
    vector<vector<set<EntityRef>>> cells;
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

    optional<uint8_t> playerAddressToMaybeId(Address address);
    Address playerIdToAddress(uint8_t playerId);
    bool getPlayerBeaconAvailable(uint8_t playerId);
    void setPlayerBeaconAvailable(uint8_t playerId, bool flag);

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

bool gameStatesAreIdentical_triggerDebugIfNot(Game* game1, Game* game2);
void triggerDebug();

#endif // ENGINE_H

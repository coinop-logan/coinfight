#include <iostream>
#include <boost/shared_ptr.hpp>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <vector>
#include <string>
#include <pthread.h>
#include "coins.h"
#include "myvectors.h"
#include "vchpack.h"
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

    void pack(vch *dest);
    void unpackAndMoveIter(vchIter *iter);

    Player(string address);
    Player(vchIter *iter);
};

void packFrameCmdsPacket(vch *dest, uint64_t frame);

struct SearchGridRect
{
    vector2i start, end;
    SearchGridRect(vector2i start, vector2i end);
};

class SearchGrid
{
    vector<EntityRef> cells[SEARCH_GRID_NUM_ROWS][SEARCH_GRID_NUM_ROWS];
public:
    SearchGrid();
    optional<vector2i> gamePosToCell(vector2f gamePos);
    void registerEntity(boost::shared_ptr<Entity> entity);
    void updateEntityCell(boost::shared_ptr<Entity> entity);
    SearchGridRect gridRectNearGamePos(vector2f gamePos, float radius);
    vector<boost::shared_ptr<Entity>> entitiesInGridRect(SearchGridRect rect);
    vector<boost::shared_ptr<Entity>> entitiesNearGamePos(vector2f gamePos, float radius);
};

class Game
{
public:
    enum State {
        Pregame,
        Active
    } state;
    uint64_t frame;
    vector<Player> players;
    vector<boost::shared_ptr<Entity>> entities;
    boost::shared_ptr<GoldPile> honeypotGoldPileIfGameStarted;

    boost::shared_ptr<Entity> entityRefToPtrOrNull(EntityRef);
    EntityRef getNextEntityRef();

    int playerAddressToIdOrNegativeOne(string address);
    string playerIdToAddress(unsigned int playerId);
    bool getPlayerBeaconAvailable(unsigned int playerId);
    void setPlayerBeaconAvailable(unsigned int playerId, bool flag);

    void killAndReplaceEntity(EntityRef, boost::shared_ptr<Entity> newEntity);

    void pack(vch *dest);
    void unpackAndMoveIter(vchIter *iter);

    Game();
    Game(vchIter *);
    // void startMatch();
    // void startMatchOrPrintError();

    void reassignEntityGamePointers();

    void iterate();
};

vector<EntityRef> entityPtrsToRefs(vector<boost::shared_ptr<Entity>>);
vector<EntityRef> entityPtrsToRefs(vector<boost::shared_ptr<Unit>>);
boost::shared_ptr<Entity> entityRefToPtrOrNull(const Game&, EntityRef);

#endif // ENGINE_H

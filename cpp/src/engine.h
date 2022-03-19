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

sf::Color playerAddressToColor(string address);

class Game;

struct Player
{
    string address;
    Coins credit;
    
    void pack(vch *dest);
    void unpackAndMoveIter(vchIter *iter);

    Player(string address);
    Player(vchIter *iter);
};

void packFrameCmdsPacket(vch *dest, uint64_t frame);

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
    string playerIdToAddress(uint playerId);

    void pack(vch *dest);
    void unpackAndMoveIter(vchIter *iter);

    Game();
    Game(vchIter *);
    void startMatch();
    void startMatchOrPrintError();

    void reassignEntityGamePointers();

    void iterate();
};

vector<EntityRef> entityPtrsToRefs(vector<boost::shared_ptr<Entity>>);
vector<EntityRef> entityPtrsToRefs(vector<boost::shared_ptr<Unit>>);
boost::shared_ptr<Entity> entityRefToPtrOrNull(const Game&, EntityRef);

#endif // ENGINE_H
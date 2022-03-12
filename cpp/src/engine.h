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

#ifndef ENGINE_H
#define ENGINE_H

using namespace std;

using vch = vector<unsigned char>;
using vchIter = vector<unsigned char>::iterator;
using EntityRef = uint16_t;

class Game;
class Gateway;

class Entity
{
public:
    Game *game;
    bool dead;
    EntityRef ref;
    vector2f pos;

    virtual unsigned char typechar();
    virtual string getTypeName();
    virtual void pack(vch *dest);
    virtual void unpackAndMoveIter(vchIter *iter, Game &game);
    virtual void go();
    virtual sf::Color getPrimaryColor();
    virtual float getRotation() { return 0; }
    void die();

    bool collidesWithPoint(vector2f);

    void packEntity(vch *destVch);
    void unpackEntityAndMoveIter(vchIter *iter);
    Entity(Game *game, EntityRef ref, vector2f pos);
    Entity(Game *game, EntityRef ref, vchIter *iter);

    vector2f getPos();
};

unsigned char getMaybeNullEntityTypechar(boost::shared_ptr<Entity>);

vector<EntityRef> entityPtrsToRefs(vector<boost::shared_ptr<Entity>>);
boost::shared_ptr<Entity> entityRefToPtrOrNull(const Game&, EntityRef);

boost::shared_ptr<Entity> unpackFullEntityAndMoveIter(vchIter *iter, unsigned char typechar, Game *game, EntityRef ref);

class Target
{
private:
    vector2f pointTarget;
    EntityRef entityTarget;
public:
    enum Type
    {
        PointTarget,
        EntityTarget
    } type;

    void pack(vch *dest);
    void unpackAndMoveIter(vchIter *iter);

    Target(vector2f);
    Target(EntityRef);
    Target(vchIter *iter);

    optional<vector2f> getPointUnlessTargetDeleted(const Game&);
    optional<EntityRef> castToEntityRef();
    optional<vector2f> castToPoint();
    boost::shared_ptr<Entity> castToEntityPtr(const Game&);
};

struct Player
{
    string address;
    Coins credit;
    
    void pack(vch *dest);
    void unpackAndMoveIter(vchIter *iter);

    Player(string address);
    Player(vchIter *iter);
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

    boost::shared_ptr<Entity> entityRefToPtrOrNull(EntityRef);
    EntityRef getNextEntityRef();

    int playerAddressToIdOrNegativeOne(string address);
    string playerIdToAddress(uint playerId);

    void pack(vch *dest);
    void unpackAndMoveIter(vchIter *iter);

    Game();
    Game(vchIter *);
    void startMatchOrPrintError();

    void reassignEntityGamePointers();

    void iterate();
};

class GoldPile : public Entity
{
public:
    Coins gold;
    void pack(vch *destVch);
    void unpackAndMoveIter(vchIter *iter);
    GoldPile(Game *, EntityRef, vector2f);
    GoldPile(Game *, EntityRef, vchIter *);
    sf::Color getPrimaryColor();

    unsigned char typechar();
    string getTypeName();
    void go();
};

class Unit : public Entity
{
public:
    int ownerId;
    Coins goldInvested;
    virtual coinsInt getCost();

    void packUnit(vch *destVch);
    void unpackUnitAndMoveIter(vchIter *iter);
    Unit(Game *, EntityRef, int, coinsInt, vector2f);
    Unit(Game *, EntityRef, vchIter *);
    sf::Color getPrimaryColor();

    coinsInt build(coinsInt attemptedAmount, Coins* fromCoins);
    bool completeBuildingInstantly(Coins* fromCoins);
    coinsInt getBuilt();
    bool isActive();
};

class Building : public Unit
{
public:
    void packBuilding(vch *destVch);
    void unpackBuildingAndMoveIter(vchIter *iter);

    Building(Game *, EntityRef, int, coinsInt, vector2f);
    Building(Game *, EntityRef, vchIter *);
};

class MobileUnit : public Unit
{
private:
    Target target;
    float targetRange;
    float rotation;

    float getRotation() { return rotation; };

    void moveTowardPoint(vector2f, float);

protected:
    void setTarget(Target _target, float range);

public:
    float angle_view;
    virtual float getSpeed();
    virtual float getRange();

    Target getTarget();

    void packMobileUnit(vch *destVch);
    void unpackMobileUnitAndMoveIter(vchIter *iter);

    void mobileUnitGo();

    void cmdMove(vector2f target);

    MobileUnit(Game *game, EntityRef ref, int ownerId, coinsInt totalCost, vector2f pos);
    MobileUnit(Game *game, EntityRef ref, vchIter *iter);
};

class Prime : public MobileUnit
{
public:
    Coins heldGold;

    enum State
    {
        Idle,
        PickupGold,
        PutdownGold,
        SendGoldThroughGateway,
        PushGoldThroughGateway,
        BuildingBuilding
    } state;

    void pack(vch *dest);
    void unpackAndMoveIter(vchIter *iter);

    Prime(Game *game, EntityRef ref, int ownerId, vector2f pos);
    Prime(Game *game, EntityRef ref, vchIter *iter);

    void cmdPickup(EntityRef);
    void cmdPutdown(Target);
    void cmdSendGoldThroughGateway(boost::shared_ptr<Gateway>);
    void cmdPushGoldThroughGateway(boost::shared_ptr<Gateway>);
    void cmdBuild(boost::shared_ptr<Building>);

    float getSpeed();
    float getRange();

    unsigned char typechar();
    string getTypeName();
    coinsInt getCost();
    void go();
};

class Gateway : public Building
{
public:
    void pack(vch *dest);
    void unpackAndMoveIter(vchIter *iter);

    Gateway(Game *game, EntityRef ref, int ownerId, vector2f pos);
    Gateway(Game *game, EntityRef ref, vchIter *iter);

    unsigned char typechar();
    string getTypeName();
    coinsInt getCost();
    void go();
};

void packFrameCmdsPacket(vch *dest, uint64_t frame);

#endif // ENGINE_H
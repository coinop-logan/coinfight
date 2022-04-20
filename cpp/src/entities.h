#include <boost/shared_ptr.hpp>
#include "common.h"

#ifndef ENTITIES_H
#define ENTITIES_H

class Game;

struct RegInfo {
    Game *game;
    EntityRef ref;
    vector2i cell;
    RegInfo(Game *game, EntityRef ref, vector2i cell);
};

class Entity
{
public:
    bool dead;
    vector2f pos;
    optional<RegInfo> maybeRegInfo;

    EntityRef getRefOrThrow();
    Game *getGameOrThrow();
    vector2i getSearchGridCellOrThrow();

    virtual unsigned char typechar();
    virtual string getTypeName();
    virtual void pack(vch *dest);
    virtual void unpackAndMoveIter(vchIter *iter, Game &game);
    virtual void go();
    virtual sf::Color getTeamOrPrimaryColor();
    virtual float getRotation() { return 0; }
    virtual vector<Coins*> getDroppableCoins();
    void die();

    bool collidesWithPoint(vector2f);

    void packEntity(vch *destVch);
    void unpackEntityAndMoveIter(vchIter *iter);
    Entity(vector2f pos);
    Entity(vchIter *iter);

    vector2f getPos();
};

unsigned char getMaybeNullEntityTypechar(boost::shared_ptr<Entity>);
enum AllianceType {
    Owned,
    Enemy,
    Neutral
};
AllianceType getAllianceType(int playerIdOrNegativeOne, boost::shared_ptr<Entity> entity);

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
    Target(boost::shared_ptr<Entity>);
    Target(vchIter *iter);

    optional<vector2f> getPointUnlessTargetDeleted(const Game&);
    optional<EntityRef> castToEntityRef();
    optional<vector2f> castToPoint();
    boost::shared_ptr<Entity> castToEntityPtr(const Game&);
};

class GoldPile : public Entity
{
public:
    Coins gold;
    vector<Coins*> getDroppableCoins();
    void pack(vch *destVch);
    void unpackAndMoveIter(vchIter *iter);
    GoldPile(vector2f);
    GoldPile(vchIter *);
    sf::Color getTeamOrPrimaryColor();

    unsigned char typechar();
    string getTypeName();
    void go();
};

class Unit : public Entity
{
    uint16_t health;
public:
    int ownerId;
    Coins goldInvested;
    vector<Coins*> getDroppableCoins();
    virtual coinsInt getCost();
    virtual uint16_t getMaxHealth();

    void packUnit(vch *destVch);
    void unpackUnitAndMoveIter(vchIter *iter);
    Unit(int, coinsInt, uint16_t, vector2f);
    Unit(vchIter *);
    sf::Color getTeamOrPrimaryColor();
    sf::Color getTeamColor();

    coinsInt build(coinsInt attemptedAmount, Coins* fromCoins);
    coinsInt unbuild(coinsInt attemptedAmount, Coins* toCoins);
    bool completeBuildingInstantly(Coins* fromCoins);
    coinsInt getBuilt();
    float getBuiltRatio();
    bool isActive();
    void unitGo();
    void takeHit(uint16_t damage);
    uint16_t getHealth();
};

class Building : public Unit
{
public:
    void packBuilding(vch *destVch);
    void unpackBuildingAndMoveIter(vchIter *iter);

    Building(int, coinsInt, uint16_t, vector2f);
    Building(vchIter *);

    void buildingGo();
};

class MobileUnit : public Unit
{
private:
    optional<pair<Target, float>> maybeTargetAndRange;

    float getRotation() { return angle_view; };

    void moveTowardPoint(vector2f, float);

public:
    void setTarget(Target _target, float range);
    float angle_view;
    virtual float getSpeed();
    virtual float getRange();
    virtual void onMoveCmd(vector2f moveTo);

    optional<Target> getTarget();

    void packMobileUnit(vch *destVch);
    void unpackMobileUnitAndMoveIter(vchIter *iter);

    void mobileUnitGo();

    void cmdMove(vector2f target);

    MobileUnit(int ownerId, coinsInt totalCost, uint16_t, vector2f pos);
    MobileUnit(vchIter *iter);
};

enum GoldTransferState {
    NoGoldTransfer,
    Pushing,
    Pulling
};

class Beacon : public Building
{
public:
    enum State {
        Spawning,
        Despawning
    } state;

    void pack(vch *dest);
    void unpackAndMoveIter(vchIter *iter);

    Beacon(int ownerId, vector2f pos, State state);
    Beacon(vchIter *iter);

    unsigned char typechar();
    string getTypeName();
    coinsInt getCost();
    uint16_t getMaxHealth();
    void go();
};

class Gateway : public Building
{
public:
    enum State {
        Idle,
        DepositTo,
        Scuttle
    } state;

    GoldTransferState inGameTransferState;

    optional<EntityRef> maybeTargetEntity;

    void pack(vch *dest);
    void unpackAndMoveIter(vchIter *iter);

    Gateway(int ownerId, vector2f pos);
    Gateway(vchIter *iter);

    void cmdBuildUnit(unsigned char unitTypechar);
    void cmdDepositTo(Target target);
    void cmdScuttle(EntityRef targetRef);
    float buildQueueWeight();

    unsigned char typechar();
    string getTypeName();
    coinsInt getCost();
    uint16_t getMaxHealth();
    void go();
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
        Build
    } state;

    GoldTransferState goldTransferState;

    unsigned char gonnabuildTypechar;

    float getSpeed();
    float getRange();
    void onMoveCmd(vector2f moveTo);

    void pack(vch *dest);
    void unpackAndMoveIter(vchIter *iter);

    Prime(int ownerId, vector2f pos);
    Prime(vchIter *iter);

    void cmdPickup(Target);
    void cmdPutdown(Target);
    void cmdBuild(unsigned char buildTypechar, vector2f buildPos);
    void cmdResumeBuilding(EntityRef targetUnit);
    void cmdScuttle(EntityRef targetUnit);

    unsigned char typechar();
    string getTypeName();
    coinsInt getCost();
    uint16_t getMaxHealth();
    void go();
    vector<Coins*> getDroppableCoins();
};

class Fighter : public MobileUnit
{
public:
    enum State
    {
        Idle,
        AttackingUnit
    } state;

    uint16_t shootCooldown;

    enum AnimateShot
    {
        None,
        Right,
        Left
    } animateShot, lastShot;

    float getSpeed();
    float getRange();
    void onMoveCmd(vector2f moveTo);

    void pack(vch *dest);
    void unpackAndMoveIter(vchIter *iter);

    Fighter(int ownerId, vector2f pos);
    Fighter(vchIter *iter);

    void cmdAttack(EntityRef ref);

    unsigned char typechar();
    string getTypename();
    coinsInt getCost();
    uint16_t getMaxHealth();
    void go();

    void shootAt(boost::shared_ptr<Unit> targetUnit);
};

boost::shared_ptr<Entity> unpackFullEntityAndMoveIter(vchIter *iter, unsigned char typechar);

#endif // ENTITIES_H
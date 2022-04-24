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
    vector2f pos;
protected:
    void setPosAndUpdateCell(vector2f newPos);
public:
    bool dead;
    optional<RegInfo> maybeRegInfo;
    vector2f getPos() const;
    virtual float getRadius() const;

    EntityRef getRefOrThrow();
    Game *getGameOrThrow();
    vector2i getSearchGridCellOrThrow();

    virtual unsigned char typechar() const;
    virtual string getTypeName() const;
    virtual void pack(vch *dest);
    virtual void unpackAndMoveIter(vchIter *iter, Game &game);
    virtual void iterate();
    virtual sf::Color getTeamOrPrimaryColor();
    virtual float getRotation() const { return 0; }
    virtual vector<Coins*> getDroppableCoins();
    void die();

    bool collidesWithPoint(vector2f);

    void packEntity(vch *destVch);
    void unpackEntityAndMoveIter(vchIter *iter);
    Entity(vector2f pos);
    Entity(vchIter *iter);
};

unsigned char getMaybeNullEntityTypechar(boost::shared_ptr<Entity>);
enum AllianceType {
    Owned,
    Foreign,
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

    bool isStillValid(const Game&);
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

    float getRadius() const;
    unsigned char typechar() const;
    string getTypeName() const;
    void iterate();
};

class Unit : public Entity
{
    uint16_t health;
public:
    int ownerId;
    Coins goldInvested;
    vector<Coins*> getDroppableCoins();
    virtual coinsInt getCost() const;
    virtual uint16_t getMaxHealth() const;

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
    void unitIterate();
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

    void buildingIterate();
};

class MobileUnit : public Unit
{
private:
    optional<pair<Target, float>> maybeTargetAndRange;
    vector2f desiredVelocity;
    vector2f lastVelocity;

    float getRotation() { return angle_view; };

    void tryMoveTowardPoint(vector2f, float);
public:
    void moveWithVelocityAndUpdateCell(vector2f toAdd);
    void setMoveTarget(Target _target, float range);
    void clearMoveTarget();
    bool isIdle();
    float angle_view;
    virtual float getMaxSpeed() const;
    vector2f getDesiredVelocity() const;
    vector2f getLastVelocity() const;
    virtual float getRange() const;
    virtual void onMoveCmd(vector2f moveTo);

    optional<Target> getMoveTarget();

    void packMobileUnit(vch *destVch);
    void unpackMobileUnitAndMoveIter(vchIter *iter);

    void mobileUnitIterate();

    void cmdMove(vector2f target);

    MobileUnit(int ownerId, coinsInt totalCost, uint16_t, vector2f pos);
    MobileUnit(vchIter *iter);
};

enum GoldTransferState {
    NoGoldTransfer,
    Pushing,
    Pulling
};

const coinsInt BEACON_BUILD_RATE = 10;
const uint16_t BEACON_HEALTH = 100;
const int BEACON_RADIUS = 10;

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

    float getRadius() const;
    unsigned char typechar() const;
    string getTypeName() const;
    coinsInt getCost() const;
    uint16_t getMaxHealth() const;
    void iterate();
};

const coinsInt GATEWAY_SCUTTLE_RATE = 5;
const coinsInt GATEWAY_COST = 4000;
const uint16_t GATEWAY_HEALTH = 1500;
const float GATEWAY_RANGE = 150;
const coinsInt GATEWAY_BUILD_RATE = 8;
const int GATEWAY_RADIUS = 15;

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

    float getRadius() const;
    unsigned char typechar() const;
    string getTypeName() const;
    coinsInt getCost() const;
    uint16_t getMaxHealth() const;
    void iterate();
};

const coinsInt PRIME_COST = 500;
const uint16_t PRIME_HEALTH = 100;
const float PRIME_SPEED = 2;
const float PRIME_TRANSFER_RANGE = 150;
const float PRIME_SIGHT_RANGE = 200;
const coinsInt PRIME_PICKUP_RATE = 2;
const coinsInt PRIME_PUTDOWN_RATE = 5;
const coinsInt PRIME_MAX_GOLD_HELD = 1000;
const int PRIME_RADIUS = 10;

class Prime : public MobileUnit
{
public:
    Coins heldGold;

    enum Behavior
    {
        Basic,
        Gather
    } behavior;

    optional<vector2f> maybeGatherTargetPos;

    enum State
    {
        NotTransferring,
        PickupGold,
        PutdownGold,
        Build
    } state;

    GoldTransferState goldTransferState;

    unsigned char gonnabuildTypechar;

    float getMaxSpeed() const;
    float getRange() const;
    void onMoveCmd(vector2f moveTo);

    void pack(vch *dest);
    void unpackAndMoveIter(vchIter *iter);

    Prime(int ownerId, vector2f pos);
    Prime(vchIter *iter);

    void cmdPickup(Target);
    void cmdPutdown(Target);
    void cmdBuild(unsigned char buildTypechar, vector2f buildPos);
    void cmdResumeBuilding(EntityRef targetUnit);
    void cmdGather(vector2f targetPos);
    void cmdScuttle(EntityRef targetUnit);

    void setStateToReturnGoldOrResetBehavior();

    float getHeldGoldRatio();

    float getRadius() const;
    unsigned char typechar() const;
    string getTypeName() const;
    coinsInt getCost() const;
    uint16_t getMaxHealth() const;
    void iterate();
    vector<Coins*> getDroppableCoins();
};

const vector2f FIGHTER_SHOT_OFFSET(20, 10);
const coinsInt FIGHTER_COST = 1500;
const uint16_t FIGHTER_HEALTH = 300;
const float FIGHTER_SPEED = 3;
const float FIGHTER_SHOT_RANGE = 200;
const float FIGHTER_SIGHT_RANGE = 250;
const int FIGHTER_SHOT_COOLDOWN = 20;
const int FIGHTER_DAMAGE = 10;
const int FIGHTER_RADIUS = 15;

class Fighter : public MobileUnit
{
public:
    enum State
    {
        NotAttacking,
        AttackingGeneral,
        AttackingSpecific
    } state;

    optional<Target> maybeAttackingGeneralTarget;

    uint16_t shootCooldown;

    enum AnimateShot
    {
        None,
        Right,
        Left
    } animateShot, lastShot;

    float getMaxSpeed() const;
    float getRange() const;
    void onMoveCmd(vector2f moveTo);

    void pack(vch *dest);
    void unpackAndMoveIter(vchIter *iter);

    Fighter(int ownerId, vector2f pos);
    Fighter(vchIter *iter);

    void cmdAttack(Target target);

    float getRadius() const;
    unsigned char typechar() const;
    string getTypename() const;
    coinsInt getCost() const;
    uint16_t getMaxHealth() const;
    void iterate();

    float calcAttackPriority(boost::shared_ptr<Unit> foreignUnit);
    void tryShootAt(boost::shared_ptr<Unit> targetUnit);
    void shootAt(boost::shared_ptr<Unit> targetUnit);
};

boost::shared_ptr<Entity> unpackFullEntityAndMoveIter(vchIter *iter, unsigned char typechar);

#endif // ENTITIES_H
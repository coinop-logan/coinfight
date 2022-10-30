#include <boost/shared_ptr.hpp>
#include "common.h"

#ifndef ENTITIES_H
#define ENTITIES_H

const fixed32 MAX_UNIT_RADIUS(30);

class Game;

struct RegInfo {
    Game *game;
    EntityRef ref;
    vector2i cell;
    RegInfo(Game *game, EntityRef ref, vector2i cell);
};

class Entity
{
    vector2fp pos;
protected:
    void setPosAndUpdateCell(vector2fp newPos);
public:
    bool dead;
    optional<RegInfo> maybeRegInfo;
    vector2fp getPos() const;
    virtual fixed32 getRadius() const;

    EntityRef getRefOrThrow();
    Game *getGameOrThrow();
    vector2i getSearchGridCellOrThrow();
    void updateGamePointerOrThrow(Game *game);

    virtual uint8_t typechar() const;
    virtual string getTypename() const;
    virtual void iterate();
    virtual sf::Color getTeamOrPrimaryColor();
    // virtual float getRotation_view() const { return 0; }
    virtual vector<Coins*> getDroppableCoins();
    virtual void pack(Netpack::Builder* to);

    void die();

    bool collidesWithPoint(vector2fp);
    bool isWithinRangeOfActiveGatewayOwnedBy(uint8_t);
protected:
    Entity(); // this will throw if called. Needed for virtual inheritance later but should never be called.
    Entity(vector2fp pos);
    void packEntityBasics(Netpack::Builder* to);
    Entity(Netpack::Consumer* from);
};

uint8_t getMaybeNullEntityTypechar(boost::shared_ptr<Entity>);
enum AllianceType {
    Owned,
    Foreign,
    Neutral
};
AllianceType getAllianceType(optional<uint8_t> maybePlayerId, boost::shared_ptr<Entity> entity);

class Target
{
private:
    vector2fp pointTarget;
    EntityRef entityTarget;
public:
    enum Type
    {
        PointTarget,
        EntityTarget
    } type;

    void pack(Netpack::Builder* to);

    Target(vector2fp);
    Target(EntityRef);
    Target(boost::shared_ptr<Entity>);
    Target(Netpack::Consumer* from);

    bool isStillValid(const Game&);
    optional<vector2fp> getPointUnlessTargetDeleted(const Game&);
    optional<EntityRef> castToEntityRef();
    optional<vector2fp> castToPoint();
    boost::shared_ptr<Entity> castToEntityPtr(const Game&);
};

class GoldPile : public Entity
{
public:
    Coins gold;
    vector<Coins*> getDroppableCoins();
    void pack(Netpack::Builder* to);
    GoldPile(vector2fp);
    GoldPile(Netpack::Consumer*);
    sf::Color getTeamOrPrimaryColor();

    fixed32 getRadius() const;
    uint8_t typechar() const;
    string getTypename() const;
    void iterate();
};

class Unit : public Entity
{
    uint16_t healthAssumingBuilt;
public:
    uint8_t ownerId;
    Coins goldInvested;
    vector<Coins*> getDroppableCoins();
    virtual coinsInt getCost() const;
    virtual uint16_t getMaxHealth() const;
    virtual fixed32 getRange() const;
    virtual bool isIdle();
    float angle_view;

    sf::Color getTeamOrPrimaryColor();
    sf::Color getTeamColor();

    coinsInt build(coinsInt attemptedAmount, Coins* fromCoins);
    coinsInt unbuild(coinsInt attemptedAmount, Coins* toCoins);
    bool completeBuildingInstantly(Coins* fromCoins);
    coinsInt getBuilt();
    fixed32 getBuiltRatio();
    bool isActive();
    void iterateUnitBasics();
    void takeHit(uint16_t damage);
    uint16_t getEffectiveHealth();
    uint16_t getHealthAssumingBuilt();

    virtual void cmdStop();

protected:
    Unit(uint8_t ownerId, coinsInt totalCost, uint16_t healthAssumingBuilt, vector2fp pos);
    void packEntityAndUnitBasics(Netpack::Builder* to);
    Unit(Netpack::Consumer*);
    Unit(); // this will throw if called. Needed for virtual inheritance later but should never be called.
};

class Building : public virtual Unit
{
public:
    Building(); // this will throw if called. Needed for virtual inheritance later but should never be called.
    Building(Netpack::Consumer*);
    void packBuildingBasics(Netpack::Builder*);
    void iterateBuildingBasics();
};

struct MoveTargetInfo
{
    Target target;
    fixed32 desiredRange;
    uint32_t closestDistanceFloorSquared;
    fixed32 frustration;

    void pack(Netpack::Builder* to);

    MoveTargetInfo(Target target, fixed32 desiredRange, uint32_t closestDistanceFloorSquared);
    MoveTargetInfo(Netpack::Consumer* from);
};

const fixed32 MOBILEUNIT_FRUSTRATION_GROWTH_FACTOR(2);

class MobileUnit : public virtual Unit
{
private:
    optional<MoveTargetInfo> maybeTargetInfo;
    vector2fp desiredVelocity;
    vector2fp lastVelocity;

    void tryMoveTowardPoint(vector2fp, fixed32);
public:
    void moveWithVelocityAndUpdateCell(vector2fp toAdd);
    void setMoveTarget(Target _target, fixed32 range);
    void clearMoveTarget();
    virtual fixed32 getMaxSpeed() const;
    vector2fp getDesiredVelocity() const;
    vector2fp getLastVelocity() const;
    virtual void onMoveCmd(vector2fp moveTo);

    optional<Target> getMaybeMoveTarget();
    optional<MoveTargetInfo> getMaybeMoveTargetInfo();
    bool mobileUnitIsIdle();

    void cmdMove(vector2fp target);
    void mobileUnitStop();

protected:
    MobileUnit(); // this will throw if called. Needed for virtual inheritance later but should never be called.
    MobileUnit(Netpack::Consumer* from);
    void packMobileUnitBasics(Netpack::Builder*);
    void iterateMobileUnitBasics();
};

class CombatUnit : public virtual Unit
{
public:
    enum State
    {
        NotAttacking,
        AttackingGeneral,
        AttackingSpecific
    } state;

    optional<Target> maybeAttackObjective;
    optional<EntityRef> maybeAttackTarget;

    uint16_t shootCooldown;

    enum AnimateShot
    {
        None,
        Right,
        Left
    } animateShot_view, lastShot_view;
    vector2fp lastShotTargetPos_view;

    bool combatUnitIsIdle();
    void combatUnitStop();

    virtual uint32_t getShotRangeFloorsquared() const;
    virtual uint16_t getShotCooldown() const;
    virtual uint16_t getShotDamage() const;
    virtual fixed32 getShotRange() const;

    virtual fixed32 getAggressionRange() const;

    void cmdAttack(Target target);
    fixed32 calcAttackPriority(boost::shared_ptr<Unit> foreignUnit);
    bool tryShootAt(boost::shared_ptr<Unit> targetUnit);
    void shootAt(boost::shared_ptr<Unit> targetUnit);

    optional<EntityRef> getMaybeTargetUnit();
protected:
    CombatUnit();
    CombatUnit(Netpack::Consumer* from);
    void packCombatUnitBasics(Netpack::Builder*);
    void iterateCombatUnitBasics();
};

enum GoldTransferState {
    NoGoldTransfer,
    Pushing,
    Pulling,
    BuildingSomething,
    ScuttlingSomething
};

const coinsInt BEACON_BUILD_RATE = 20;
// const uint16_t BEACON_HEALTH = 100;
const fixed32 BEACON_RADIUS(10);

class Beacon : public Building
{
public:
    enum State {
        Spawning,
        Despawning
    } state;

    void pack(Netpack::Builder* to);

    Beacon(uint8_t ownerId, vector2fp pos, State state);
    Beacon(Netpack::Consumer* from);

    fixed32 getRadius() const;
    uint8_t typechar() const;
    string getTypename() const;
    coinsInt getCost() const;
    uint16_t getMaxHealth() const;

    bool isIdle() { return false; }
    void cmdStop();

    void iterate();
};

const coinsInt GATEWAY_COST = 4000;
const uint16_t GATEWAY_HEALTH = 1500;
const fixed32 GATEWAY_RANGE(150);
const uint32_t GATEWAY_RANGE_FLOORSQUARED = floorSquareFixed(GATEWAY_RANGE);
const coinsInt GATEWAY_SCUTTLE_RATE = 2;
const coinsInt GATEWAY_BUILD_RATE = 4;
const fixed32 GATEWAY_RADIUS(15); // don't forget to update MAX_UNIT_RADIUS!!

class Gateway : public Building
{
public:
    vector<EntityRef> buildTargetQueue;
    vector<EntityRef> scuttleTargetQueue;
    bool pushing_view, building_view, pulling_view, scuttling_view;

    void pack(Netpack::Builder* to);

    Gateway(uint8_t ownerId, vector2fp pos);
    Gateway(Netpack::Consumer* from);

    void cmdBuildUnit(uint8_t unitTypechar);
    void cmdDepositTo(Target target);
    void cmdScuttle(EntityRef targetRef);
    void cmdStopScuttle(EntityRef targetRef);
    fixed32 buildQueueWeight();

    bool isIdle();
    void cmdStop();
    void removeFromQueues(EntityRef);

    fixed32 getRadius() const;
    uint8_t typechar() const;
    string getTypename() const;
    coinsInt getCost() const;
    uint16_t getMaxHealth() const;
    void iterate();
};

const coinsInt PRIME_COST = 500;
const uint16_t PRIME_HEALTH = 100;
const fixed32 PRIME_SPEED(2);
const fixed32 PRIME_TRANSFER_RANGE(150);
const uint32_t PRIME_TRANSFER_RANGE_FLOORSQUARED = floorSquareFixed(PRIME_TRANSFER_RANGE);
const fixed32 PRIME_SIGHT_RANGE(200);
const coinsInt PRIME_PICKUP_RATE = 2;
const coinsInt PRIME_PUTDOWN_RATE = 4;
const coinsInt PRIME_MAX_GOLD_HELD = 200;
const fixed32 PRIME_RADIUS(10); // don't forget to update MAX_UNIT_RADIUS!!

class Prime : public MobileUnit
{
public:
    Coins heldGold;

    enum Behavior
    {
        Basic,
        Gather
    } behavior;

    optional<vector2fp> maybeGatherTargetPos;

    enum State
    {
        NotTransferring,
        PickupGold,
        PutdownGold,
        Build
    } state;

    GoldTransferState goldTransferState_view;

    bool depositingToGateway;

    uint8_t gonnabuildTypechar;

    fixed32 getMaxSpeed() const;
    fixed32 getRange() const;
    void onMoveCmd(vector2fp moveTo);

    void pack(Netpack::Builder* to);

    Prime(uint8_t ownerId, vector2fp pos);
    Prime(Netpack::Consumer* from);

    void cmdPickup(Target);
    void cmdPutdown(Target);
    void cmdBuild(uint8_t buildTypechar, vector2fp buildPos);
    void cmdResumeBuilding(EntityRef targetUnit);
    void cmdGather(vector2fp targetPos);
    void cmdScuttle(EntityRef targetUnit);

    void setStateToReturnGoldOrResetBehavior();

    fixed32 getHeldGoldRatio();

    bool isIdle();
    void cmdStop();

    fixed32 getRadius() const;
    uint8_t typechar() const;
    string getTypename() const;
    coinsInt getCost() const;
    uint16_t getMaxHealth() const;
    void iterate();
    vector<Coins*> getDroppableCoins();
};

const coinsInt FIGHTER_COST = 1500;
const uint16_t FIGHTER_HEALTH = 300;
const fixed32 FIGHTER_SPEED(3);
const fixed32 FIGHTER_SHOT_RANGE(200);
const uint32_t FIGHTER_SHOT_RANGE_FLOORSQUARED = floorSquareFixed(FIGHTER_SHOT_RANGE);
const fixed32 FIGHTER_AGGRESSION_RANGE(250);
const uint8_t FIGHTER_SHOT_COOLDOWN = 20;
const uint8_t FIGHTER_SHOT_DAMAGE = 10;
const fixed32 FIGHTER_RADIUS(15); // don't forget to update MAX_UNIT_RADIUS!!

class Fighter : public MobileUnit, public CombatUnit
{
public:
    fixed32 getMaxSpeed() const;
    fixed32 getRange() const;
    void onMoveCmd(vector2fp moveTo);

    void pack(Netpack::Builder* to);

    Fighter(uint8_t ownerId, vector2fp pos);
    Fighter(Netpack::Consumer* from);
    
    bool isIdle();
    void cmdStop();

    uint32_t getShotRangeFloorsquared() const;
    uint16_t getShotCooldown() const;
    uint16_t getShotDamage() const;
    fixed32 getShotRange() const;

    fixed32 getRadius() const;
    uint8_t typechar() const;
    string getTypename() const;
    coinsInt getCost() const;
    uint16_t getMaxHealth() const;
    fixed32 getAggressionRange() const;
    void iterate();
};

const coinsInt TURRET_COST = 5000;
const uint16_t TURRET_HEALTH = 900;
const fixed32 TURRET_RADIUS(30); // don't forget to update MAX_UNIT_RADIUS!!
const fixed32 TURRET_SHOT_RANGE(400);
const uint32_t TURRET_SHOT_RANGE_FLOORSQUARED = floorSquareFixed(TURRET_SHOT_RANGE);
const uint8_t TURRET_SHOT_COOLDOWN = 50;
const uint8_t TURRET_SHOT_DAMAGE = 100;

class Turret : public Building, public CombatUnit
{
public:
    void pack(Netpack::Builder* to);

    Turret(uint8_t ownerId, vector2fp pos);
    Turret(Netpack::Consumer* from);

    bool isIdle();
    void cmdStop();

    uint32_t getShotRangeFloorsquared() const;
    uint16_t getShotCooldown() const;
    uint16_t getShotDamage() const;
    fixed32 getShotRange() const;

    fixed32 getRange() const;
    fixed32 getRadius() const;
    uint8_t typechar() const;
    string getTypename() const;
    coinsInt getCost() const;
    uint16_t getMaxHealth() const;
    fixed32 getAggressionRange() const;
    void iterate();
};

boost::shared_ptr<Entity> maybeEntityRefToPtrOrNull(const Game&, optional<EntityRef>);

boost::shared_ptr<Entity> consumeEntity(Netpack::Consumer* from);

bool entitiesAreIdentical_triggerDebugIfNot(boost::shared_ptr<Entity> entity1, boost::shared_ptr<Entity> entity2);

#endif // ENTITIES_H
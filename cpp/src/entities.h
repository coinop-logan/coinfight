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
public:
    void setPosWithoutUpdatingCell(vector2fp newPos);
    void setPosAndUpdateCell(vector2fp newPos);
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

    bool operator==(const Target& other) const;

    bool isStillValid(const Game&);
    optional<vector2fp> getPointUnlessTargetDeleted(const Game&);
    optional<EntityRef> castToEntityRef();
    optional<vector2fp> castToPoint();
    boost::shared_ptr<Entity> castToEntityPtr(const Game&);
};

class GoldPile : public Entity
{
public:
    fixed32 getRadius() const;
    uint8_t typechar() const;
    string getTypename() const;

    Coins gold;

    GoldPile(vector2fp);
    void pack(Netpack::Builder* to);
    GoldPile(Netpack::Consumer*);

    void iterate();

    sf::Color getTeamOrPrimaryColor();
    vector<Coins*> getDroppableCoins();
};

class Unit : public Entity
{
    GameSettings* gameSettings;
    coinsInt* cost;
    uint16_t* maxHealth;
    uint16_t healthAssumingBuilt;
public:
    void setHealthAssumingBuilt(uint16_t newHealth);
    uint8_t ownerId;
    Coins goldInvested;

    float angle_view;

    vector<Coins*> getDroppableCoins();

    coinsInt getCost() { return *cost; }
    uint16_t getMaxHealth() { return *maxHealth; }
    GameSettings* getGameSettings() { return gameSettings; }

    sf::Color getTeamOrPrimaryColor();
    sf::Color getTeamColor();

    coinsInt build(coinsInt attemptedAmount, Coins* fromCoins);
    coinsInt unbuild(coinsInt attemptedAmount, Coins* toCoins);
    bool completeBuildingInstantly(Coins* fromCoins);
    coinsInt getBuilt();
    float getBuiltRatio();
    bool isFullyBuilt();
    bool isActive();
    void iterateUnitBasics();
    void takeHit(uint16_t damage);
    uint16_t getEffectiveHealth();
    uint16_t getHealthAssumingBuilt();

    virtual void cmdStop();

protected:
    Unit(GameSettings* gameSettings, uint8_t ownerId, coinsInt* totalCost, uint16_t* maxHealth, vector2fp pos);
    void packEntityAndUnitBasics(Netpack::Builder* to);
    Unit(GameSettings*, Netpack::Consumer*);
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
    virtual void onMoveFinished(Target target);

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
    virtual uint32_t getShotRangeFloorsquared() const;
    virtual uint16_t getShotCooldown() const;
    virtual uint16_t getShotDamage() const;
    virtual fixed32 getShotRange() const;
    virtual fixed32 getAggressionRange() const;

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

    fixed32 calcAttackPriority(boost::shared_ptr<Unit> foreignUnit);

    void combatUnitStop();
    void shootAt(boost::shared_ptr<Unit> targetUnit);
    bool tryShootAt(boost::shared_ptr<Unit> targetUnit);
    void cmdAttack(Target target);

    optional<EntityRef> getMaybeTargetUnit();

    bool combatUnitIsIdle();
protected:
    CombatUnit();
    void packCombatUnitBasics(Netpack::Builder*);
    CombatUnit(Netpack::Consumer* from);

    void iterateCombatUnitBasics();
};

const fixed32 BEACON_RADIUS(10);

class Beacon : public Building
{
    bool spendingTicket;
public:
    fixed32 getRadius() const;
    uint8_t typechar() const;
    string getTypename() const;

    enum State {
        Spawning,
        Despawning
    } state;

    Beacon(GameSettings* gameSettings, uint8_t ownerId, vector2fp pos, State state, bool spendingTicket);
    void pack(Netpack::Builder* to);
    Beacon(GameSettings* gameSettings, Netpack::Consumer* from);

    void cmdStop();
    void cmdWarpOut();

    void iterate();
};

const fixed32 GATEWAY_RANGE(150);
const uint32_t GATEWAY_RANGE_FLOORSQUARED = floorSquareFixed(GATEWAY_RANGE);
const fixed32 GATEWAY_RADIUS(15); // don't forget to update MAX_UNIT_RADIUS!!

const fixed32 GATEWAY_SHOT_RANGE(350);
const fixed32 GATEWAY_AGGRESSION_RANGE(400);
const uint32_t GATEWAY_SHOT_RANGE_FLOORSQUARED = floorSquareFixed(GATEWAY_SHOT_RANGE);
const uint16_t GATEWAY_SHOT_COOLDOWN = 15;
const uint16_t GATEWAY_SHOT_DAMAGE = 7;

class Prime;

class Gateway : public Building, public CombatUnit
{
public:
    fixed32 getRadius() const;
    uint8_t typechar() const;
    string getTypename() const;

    uint32_t getShotRangeFloorsquared() const;
    uint16_t getShotCooldown() const;
    uint16_t getShotDamage() const;
    fixed32 getShotRange() const;
    fixed32 getAggressionRange() const;

    optional<EntityRef> maybeDepositingPrime;
    optional<EntityRef> maybeWithdrawingPrime;

    vector<EntityRef> buildTargetQueue;
    vector<EntityRef> scuttleTargetQueue;

    bool isInBuildTargetQueue(EntityRef);
    bool isInScuttleTargetQueue(EntityRef);
    
    optional<tuple<EntityRef, bool>> getMaybeScuttleTarget();
    optional<tuple<EntityRef, bool>> getMaybeDepositTarget();

    tuple<boost::shared_ptr<Entity>, bool> goldFlowFrom_view;
    tuple<boost::shared_ptr<Entity>, bool> goldFlowTo_view;

    Gateway(GameSettings* gameSettings, uint8_t ownerId, vector2fp pos);
    void pack(Netpack::Builder* to);
    Gateway(GameSettings* gameSettings, Netpack::Consumer* from);

    void cmdBuildUnit(uint8_t unitTypechar);
    void cmdDepositTo(EntityRef entityRef);
    void cmdScuttle(EntityRef targetRef);
    void cmdStopScuttle(EntityRef targetRef);
    void cmdStop();

    bool requestDepositFromPrime(Prime*);
    bool requestWithdrawFromPrime(Prime*);
    void cancelAnyDepositRequestFromPrime(Prime*);
    void cancelAnyWithdrawRequestFromPrime(Prime*);

    void removeFromQueues(EntityRef);
    void validateTargets();
    void iterate();

    coinsInt buildQueueWeight();
    coinsInt scuttleQueueWeight();
};

const fixed32 PRIME_SPEED(1.6);
const fixed32 PRIME_TRANSFER_RANGE(150);
const uint32_t PRIME_TRANSFER_RANGE_FLOORSQUARED = floorSquareFixed(PRIME_TRANSFER_RANGE);
const fixed32 PRIME_SIGHT_RANGE(200);
const fixed32 PRIME_RADIUS(10); // don't forget to update MAX_UNIT_RADIUS!!

class Prime : public MobileUnit
{
public:
    fixed32 getRadius() const;
    uint8_t typechar() const;
    string getTypename() const;
    fixed32 getMaxSpeed() const;

    Coins heldGold;

    optional<EntityRef> fundsSource;
    optional<EntityRef> fundsDest;

    vector<Target> scavengeTargetQueue;
    vector<EntityRef> buildTargetQueue;

    optional<EntityRef> fetchToImmediateTarget;

    bool isInScavengeTargetQueue(Target);
    bool isInBuildTargetQueue(EntityRef);

    optional<tuple<Target, bool>> getMaybeFetchTarget();
    optional<tuple<Target, bool>> getMaybeImmediateFetchTarget();
    optional<tuple<EntityRef, bool>> getMaybeDepositTarget();

    tuple<boost::shared_ptr<Entity>, bool> goldFlowFrom_view;
    tuple<boost::shared_ptr<Entity>, bool> goldFlowTo_view;

    Prime(GameSettings* gameSettings, uint8_t ownerId, vector2fp pos);
    void pack(Netpack::Builder* to);
    Prime(GameSettings* gameSettings, Netpack::Consumer* from);

    void addToScavengeQueue_enforceUnique(Target, bool asap);
    void addToBuildQueue_enforceUnique(EntityRef, bool asap);

    void cmdDeposit(EntityRef, bool asap);
    void cmdFetch(Target, bool asap);
    void cmdScuttle(boost::shared_ptr<Entity>, bool asap);
    void cmdStop();
    void onMoveCmd(vector2fp moveTo);
    void onMoveFinished(Target target);

    void cancelAnyFetchesFrom(Target);
    void cancelAnyDepositsTo(Target);
    void validateTargets();
    void tryTransferAndMaybeMoveOn();
    void iterate();

    float getHeldGoldRatio();
    vector<Coins*> getDroppableCoins();
};

const fixed32 FIGHTER_SPEED(2);
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
    uint32_t getShotRangeFloorsquared() const;
    uint16_t getShotCooldown() const;
    uint16_t getShotDamage() const;
    fixed32 getShotRange() const;
    fixed32 getRadius() const;
    uint8_t typechar() const;
    string getTypename() const;
    fixed32 getAggressionRange() const;

    Fighter(GameSettings* gameSettings, uint8_t ownerId, vector2fp pos);
    void pack(Netpack::Builder* to);
    Fighter(GameSettings* gameSettings, Netpack::Consumer* from);
    
    void cmdStop();
    void onMoveCmd(vector2fp moveTo);
    void onMoveFinished(Target target);

    void iterate();
};

const fixed32 TURRET_RADIUS(30); // don't forget to update MAX_UNIT_RADIUS!!
const fixed32 TURRET_SHOT_RANGE(450);
const uint32_t TURRET_SHOT_RANGE_FLOORSQUARED = floorSquareFixed(TURRET_SHOT_RANGE);
const uint8_t TURRET_SHOT_COOLDOWN = 50;
const uint8_t TURRET_SHOT_DAMAGE = 100;

class Turret : public Building, public CombatUnit
{
public:
    uint32_t getShotRangeFloorsquared() const;
    uint16_t getShotCooldown() const;
    uint16_t getShotDamage() const;
    fixed32 getShotRange() const;
    fixed32 getRadius() const;
    uint8_t typechar() const;
    string getTypename() const;
    fixed32 getAggressionRange() const;

    Turret(GameSettings* gameSettings, uint8_t ownerId, vector2fp pos);
    void pack(Netpack::Builder* to);
    Turret(GameSettings* gameSettings, Netpack::Consumer* from);

    void cmdStop();

    void iterate();
};

boost::shared_ptr<Entity> maybeEntityRefToPtrOrNull(const Game&, optional<EntityRef>);

optional<boost::shared_ptr<Entity>> consumeEntity(GameSettings* gameSettings, Netpack::Consumer* from);

bool entitiesAreIdentical_triggerDebugIfNot(boost::shared_ptr<Entity> entity1, boost::shared_ptr<Entity> entity2);

#endif // ENTITIES_H
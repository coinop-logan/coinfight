#include <boost/shared_ptr.hpp>
#include <boost/function.hpp>
#include <boost/bind.hpp>
#include "engine.h"
#include "entities.h"
#include "fpm/math.hpp"




// ------------------------------------------------------------------------------
// ------------------------------------------------------------------------------
// ------------------------------------------------------------------------------
// ------------------------------------------------------------------------------
// ------------------------------------------------------------------------------
// ------------------------------------------------------------------------------
// ------------------------------------------------------------------------------
// ------------------------------------------------------------------------------
// ------------------------------------------------------------------------------
// ------- TARGET -------
// ------------------------------------------------------------------------------
// ------------------------------------------------------------------------------
// ------------------------------------------------------------------------------
// ------------------------------------------------------------------------------
// ------------------------------------------------------------------------------
// ------------------------------------------------------------------------------
// ------------------------------------------------------------------------------
// ------------------------------------------------------------------------------
// ------------------------------------------------------------------------------



Target::Target(vector2fp _pointTarget)
{
    type = PointTarget;
    pointTarget = _pointTarget;
}
Target::Target(EntityRef _entityTarget)
{
    type = EntityTarget;
    pointTarget = vector2fp::zero;
    entityTarget = _entityTarget;
}
Target::Target(boost::shared_ptr<Entity> entity)
    : Target(entity->getRefOrThrow()) {}
void Target::pack(Netpack::Builder* to)
{
    to->packEnum(type);
    packVector2fp(to, pointTarget);
    packEntityRef(to, entityTarget);
}
Target::Target(Netpack::Consumer* from)
{
    type = from->consumeEnum<Target::Type>();
    pointTarget = consumeVector2fp(from);
    entityTarget = consumeEntityRef(from);
}

bool Target::operator==(const Target& other) const
{
    if (this->type == other.type)
    {
        if (this->type == PointTarget)
        {
            return this->pointTarget == other.pointTarget;
        }
        else
        {
            return this->entityTarget == other.entityTarget;
        }
    }
    else
    {
        return false;
    }
}

bool Target::isStillValid(const Game &game)
{
    return
        (type == PointTarget) ||
        (maybeEntityRefToPtrOrNull(game, {entityTarget}) != NULL);
}
optional<vector2fp> Target::getPointUnlessTargetDeleted(const Game &game)
{
    if (type == PointTarget)
        return {pointTarget};
    else
        if (boost::shared_ptr<Entity> e = maybeEntityRefToPtrOrNull(game, entityTarget))
            return {e->getPos()};
    return {};
}

optional<EntityRef> Target::castToEntityRef()
{
    if (type == PointTarget)
    {
        return {};
    }
    else
    {
        return {entityTarget};
    }
}

optional<vector2fp> Target::castToPoint()
{
    if (type == PointTarget)
    {
        return {pointTarget};
    }
    else
    {
        return {};
    }
}

boost::shared_ptr<Entity> Target::castToEntityPtr(const Game &game)
{
    if (auto eRef = castToEntityRef())
        return maybeEntityRefToPtrOrNull(game, *eRef);
    else
        return boost::shared_ptr<Entity>();
}




// ------------------------------------------------------------------------------
// ------------------------------------------------------------------------------
// ------------------------------------------------------------------------------
// ------------------------------------------------------------------------------
// ------------------------------------------------------------------------------
// ------------------------------------------------------------------------------
// ------------------------------------------------------------------------------
// ------------------------------------------------------------------------------
// ------------------------------------------------------------------------------
// ------- MISC ---------
// ------------------------------------------------------------------------------
// ------------------------------------------------------------------------------
// ------------------------------------------------------------------------------
// ------------------------------------------------------------------------------
// ------------------------------------------------------------------------------
// ------------------------------------------------------------------------------
// ------------------------------------------------------------------------------
// ------------------------------------------------------------------------------
// ------------------------------------------------------------------------------

bool debugAssert(bool test)
{
    if (!test)
    {
        triggerDebug();
    }
    return test;
}

bool targetsAreEqual(Target t1, Target t2)
{
    return (
        t1.type == t2.type
     && t1.castToEntityRef() == t2.castToEntityRef()
     && t1.castToPoint() == t2.castToPoint()
    );
}

bool maybeTargetsAreEqual(optional<Target> mt1, optional<Target> mt2)
{
    if ((!mt1) && (!mt2))
    {
        return true;
    }
    if ((!mt1) && mt2)
    {
        return false;
    }
    if ((!mt2) && mt1)
    {
        return false;
    }

    return targetsAreEqual(*mt1, *mt2);
}

bool maybeMoveTargetInfosAreEqual(optional<MoveTargetInfo> mt1, optional<MoveTargetInfo> mt2)
{
    if ((!mt1) && (!mt2))
    {
        return true;
    }
    if ((!mt1) && mt2)
    {
        return false;
    }
    if ((!mt2) && mt1)
    {
        return false;
    }

    // we now know both are non-nothing
    return (
        targetsAreEqual(mt1->target, mt2->target)
     && mt1->desiredRange == mt2->desiredRange
     && mt1->closestDistanceFloorSquared == mt2->closestDistanceFloorSquared
     && mt1->frustration == mt2->frustration
    );
}

bool entitiesAreIdentical_triggerDebugIfNot(boost::shared_ptr<Entity> entity1, boost::shared_ptr<Entity> entity2)
{
    triggerDebug(); return false;
    // if (!entity1 && !entity2)
    // {
    //     // both null, good!
    //     return true;
    // }
    // if (!entity1 && entity2)
    // {
    //     triggerDebug();
    //     return false;
    // }
    // if (entity1 && !entity2)
    // {
    //     triggerDebug();
    //     return false;
    // }

    // // we now know they're both non-null

    // debugAssert(entity1->getPos() == entity2->getPos());
    // debugAssert(entity1->dead == entity2->dead);
    // // don't need to test regInfo directly
    // debugAssert(entity1->getSearchGridCellOrThrow() == entity2->getSearchGridCellOrThrow());
    // debugAssert(entity1->typechar() == entity2->typechar());

    // bool successfulCast = false; // until proven otherwise
    // if (auto goldpile1 = boost::dynamic_pointer_cast<GoldPile, Entity>(entity1))
    //     if (auto goldpile2 = boost::dynamic_pointer_cast<GoldPile, Entity>(entity2))
    // {
    //     successfulCast = true;
    //     debugAssert(goldpile1->gold.getInt() == goldpile2->gold.getInt());
    // }

    // if (auto unit1 = boost::dynamic_pointer_cast<Unit, Entity>(entity1))
    //     if (auto unit2 = boost::dynamic_pointer_cast<Unit, Entity>(entity2))
    // {
    //     debugAssert(unit1->getHealthAssumingBuilt() == unit2->getHealthAssumingBuilt());
    //     debugAssert(unit1->ownerId == unit2->ownerId);
    //     debugAssert(unit1->goldInvested.getInt() == unit2->goldInvested.getInt());

    //     if (auto building1 = boost::dynamic_pointer_cast<Building, Unit>(unit1))
    //         if (auto building2 = boost::dynamic_pointer_cast<Building, Unit>(unit2))
    //     {
    //         // no building-specific state to check

    //         if (auto beacon1 = boost::dynamic_pointer_cast<Beacon, Building>(building1))
    //             if (auto beacon2 = boost::dynamic_pointer_cast<Beacon, Building>(building2))
    //         {
    //             successfulCast = true;

    //             debugAssert(beacon1->state == beacon2->state);
    //         }

    //         if (auto gateway1 = boost::dynamic_pointer_cast<Gateway, Building>(building1))
    //             if (auto gateway2 = boost::dynamic_pointer_cast<Gateway, Building>(building2))
    //         {
    //             successfulCast = true;

    //             debugAssert(gateway1->buildTargetQueue == gateway2->buildTargetQueue);
    //             debugAssert(gateway1->scuttleTargetQueue == gateway2->scuttleTargetQueue);
    //         }

    //         if (auto turret1 = boost::dynamic_pointer_cast<Turret, Building>(building1))
    //             if (auto turret2 = boost::dynamic_pointer_cast<Turret, Building>(building2))
    //         {
    //             successfulCast = true;

    //             debugAssert(turret1->state == turret2->state);
    //             debugAssert(maybeTargetsAreEqual(turret1->maybeAttackObjective, turret2->maybeAttackObjective));
    //         }

    //         debugAssert(successfulCast);
    //     }

    //     if (auto mobileUnit1 = boost::dynamic_pointer_cast<MobileUnit, Unit>(unit1))
    //         if (auto mobileUnit2 = boost::dynamic_pointer_cast<MobileUnit, Unit>(unit2))
    //     {
    //         debugAssert(maybeMoveTargetInfosAreEqual(mobileUnit1->getMaybeMoveTargetInfo(), mobileUnit2->getMaybeMoveTargetInfo()));
    //         debugAssert(mobileUnit1->getDesiredVelocity() == mobileUnit2->getDesiredVelocity());
    //         debugAssert(mobileUnit1->getLastVelocity() == mobileUnit2->getLastVelocity());

    //         if (auto prime1 = boost::dynamic_pointer_cast<Prime, MobileUnit>(mobileUnit1))
    //             if (auto prime2 = boost::dynamic_pointer_cast<Prime, MobileUnit>(mobileUnit2))
    //         {
    //             successfulCast = true;

    //             debugAssert(prime1->heldGold.getInt() == prime2->heldGold.getInt());
    //             debugAssert(prime1->behavior == prime2->behavior);
    //             debugAssert(prime1->maybeGatherTargetPos == prime2->maybeGatherTargetPos);
    //             debugAssert(prime1->state == prime2->state);
    //             debugAssert(prime1->gonnabuildTypechar == prime2->gonnabuildTypechar);
    //         }

    //         if (auto fighter1 = boost::dynamic_pointer_cast<Fighter, MobileUnit>(mobileUnit1))
    //             if (auto fighter2 = boost::dynamic_pointer_cast<Fighter, MobileUnit>(mobileUnit2))
    //         {
    //             successfulCast = true;

    //             debugAssert(fighter1->state == fighter2->state);
    //             debugAssert(maybeTargetsAreEqual(fighter1->maybeAttackObjective, fighter2->maybeAttackObjective));
    //         }

    //         debugAssert(successfulCast);
    //     }

    //     debugAssert(successfulCast);
    // }

    // return debugAssert(successfulCast);
    // // this repetition of debugAssert may seem redundant,
    // // but this way, breakpoints trigger in a more informative position
}


RegInfo::RegInfo(Game *game, EntityRef ref, vector2i cell)
    : game(game), ref(ref), cell(cell) {}

vector<EntityRef> entityPtrsToRefsOrThrow(vector<boost::shared_ptr<Entity>> ptrs)
{
    vector<EntityRef> refs;
    for (unsigned int i = 0; i < ptrs.size(); i++)
    {
        refs.push_back(ptrs[i]->getRefOrThrow());
    }
    return refs;
}

vector<EntityRef> entityPtrsToRefsOrThrow(vector<boost::shared_ptr<Unit>> unitPtrs)
{
    vector<EntityRef> refs;
    for (unsigned int i=0; i<unitPtrs.size(); i++)
    {
        refs.push_back(unitPtrs[i]->getRefOrThrow());
    }
    return refs;
}

boost::shared_ptr<Entity> maybeEntityRefToPtrOrNull(const Game& game, optional<EntityRef> maybeRef)
{
    if (auto ref = maybeRef)
    {
        return game.entities[*ref];
    }
    else
    {
        return boost::shared_ptr<Entity>();
    }
}

uint8_t getMaybeNullEntityTypechar(boost::shared_ptr<Entity> e)
{
    if (e)
        return e->typechar();
    else
        return NULL_TYPECHAR;
}

AllianceType getAllianceType(optional<uint8_t> maybePlayerId, boost::shared_ptr<Entity> entity)
{
    if (auto goldPile = boost::dynamic_pointer_cast<GoldPile, Entity>(entity))
    {
        return Neutral;
    }
    else if (auto unit = boost::dynamic_pointer_cast<Unit, Entity>(entity))
    {
        if ((maybePlayerId) && *maybePlayerId == unit->ownerId)
        {
            return Owned;
        }
        else
        {
            return Foreign;
        }
    }
    else
    {
        cout << "Not sure how to getAllianceType for that. Defaulting to neutral." << endl;
        return Neutral;
    }
}




// ------------------------------------------------------------------------------
// ------------------------------------------------------------------------------
// ------------------------------------------------------------------------------
// ------------------------------------------------------------------------------
// ------------------------------------------------------------------------------
// ------------------------------------------------------------------------------
// ------------------------------------------------------------------------------
// ------------------------------------------------------------------------------
// ------------------------------------------------------------------------------
// ------- ENTITY -------
// ------------------------------------------------------------------------------
// ------------------------------------------------------------------------------
// ------------------------------------------------------------------------------
// ------------------------------------------------------------------------------
// ------------------------------------------------------------------------------
// ------------------------------------------------------------------------------
// ------------------------------------------------------------------------------
// ------------------------------------------------------------------------------
// ------------------------------------------------------------------------------


vector2fp Entity::getPos() const
{
    return pos;
}
fixed32 Entity::getRadius() const
{
    throw runtime_error("getRadius() has not been defined for " + getTypename() + ".\n");
}
void Entity::setPosAndUpdateCell(vector2fp newPos)
{
    pos = newPos;
    getGameOrThrow()->searchGrid.updateEntityCellRelation(this);
}
Game* Entity::getGameOrThrow()
{
    if (auto regInfo = maybeRegInfo)
    {
        return regInfo->game;
    }
    else
    {
        throw runtime_error("Trying to get a ref for an entity that has not been registered with a Game.\n");
    }
}
EntityRef Entity::getRefOrThrow()
{
    if (auto regInfo = maybeRegInfo)
    {
        return regInfo->ref;
    }
    else
    {
        throw runtime_error("Trying to get a ref for an entity that has not been registered with a Game.\n");
    }
}
vector2i Entity::getSearchGridCellOrThrow()
{
    if (auto regInfo = maybeRegInfo)
    {
        return regInfo->cell;
    }
    else
    {
        throw runtime_error("Trying to get a searchGridCell for an entity that has not been registered with a Game.\n");
    }
}
void Entity::updateGamePointerOrThrow(Game *game)
{
    if (maybeRegInfo)
    {
        maybeRegInfo->game = game;
    }
    else
    {
        throw runtime_error("Trying to update the Game pointer for an entity that has not been registered within a Game.\n");
    }
}
uint8_t Entity::typechar() const
{
    throw runtime_error("typechar() has not been defined for " + getTypename() + "\n");
}
string Entity::getTypename() const
{
    throw runtime_error("getTypename() has not been defined for this unit.\n");
}

bool Entity::collidesWithPoint(vector2fp point)
{
    return (pos - point).getRoughMagnitude() <= ENTITY_COLLIDE_RADIUS;
}
bool Entity::isWithinRangeOfActiveGatewayOwnedBy(uint8_t targetOwnerId) {
    Game *game = getGameOrThrow();

    auto entities = game->entitiesWithinCircle(getPos(), GATEWAY_RANGE);
    for (unsigned int i=0; i<entities.size(); i++)
    {
        if (auto gateway = boost::dynamic_pointer_cast<Gateway, Entity>(entities[i]))
        {
            if (gateway->isActive() && gateway->ownerId == targetOwnerId)
            {
                return true;
            }
        }
    }
    return false;
}

void Entity::iterate()
{
    throw runtime_error("go() has not been defined for " + getTypename() + ".\n");
}
sf::Color Entity::getTeamOrPrimaryColor()
{
    throw runtime_error("getTeamOrPrimaryColor() has not been defined for " + getTypename() + ".\n");
}
void Entity::pack(Netpack::Builder* to)
{
    throw runtime_error("pack() has not been defined for " + getTypename() + ".\n");
}
Entity::Entity()
{
    throw logic_error("Default constructor for Entity called. This should never happen! Check virtual inheritance structure.\n");
}
Entity::Entity(vector2fp pos) : pos(pos),
                                dead(false)
{}
void Entity::packEntityBasics(Netpack::Builder* to)
{
    to->packBool(dead);
    packVector2fp(to, pos);
}
Entity::Entity(Netpack::Consumer* from)
{
    dead = from->consumeBool();
    pos = consumeVector2fp(from);
}
void Entity::die()
{
    dead = true;
}
vector<Coins*> Entity::getDroppableCoins()
{
    throw runtime_error("getDroppableCoins has not been defined for " + getTypename() + ".");
}



// ------------------------------------------------------------------------------
// ------------------------------------------------------------------------------
// ------------------------------------------------------------------------------
// ------------------------------------------------------------------------------
// ------------------------------------------------------------------------------
// ------------------------------------------------------------------------------
// ------------------------------------------------------------------------------
// ------------------------------------------------------------------------------
// ------------------------------------------------------------------------------
// ------- GOLDPILE -----
// ------------------------------------------------------------------------------
// ------------------------------------------------------------------------------
// ------------------------------------------------------------------------------
// ------------------------------------------------------------------------------
// ------------------------------------------------------------------------------
// ------------------------------------------------------------------------------
// ------------------------------------------------------------------------------
// ------------------------------------------------------------------------------
// ------------------------------------------------------------------------------



vector<Coins*> GoldPile::getDroppableCoins()
{
    return vector<Coins*>{&gold};
}
sf::Color GoldPile::getTeamOrPrimaryColor()
{
    return sf::Color(sf::Color::Yellow);
}

GoldPile::GoldPile(vector2fp pos) : Entity(pos),
                                   gold(MAX_COINS)
{}
void GoldPile::pack(Netpack::Builder* to)
{
    packEntityBasics(to);
    gold.pack(to);
}
GoldPile::GoldPile(Netpack::Consumer* from)
    : Entity(from)
{
    gold = Coins(from);
}

fixed32 GoldPile::getRadius() const { return fixed32(10); }
uint8_t GoldPile::typechar() const { return GOLDPILE_TYPECHAR; }
string GoldPile::getTypename() const { return "GoldPile"; }
void GoldPile::iterate()
{
    if (gold.getInt() == 0)
        die();
}



// ------------------------------------------------------------------------------
// ------------------------------------------------------------------------------
// ------------------------------------------------------------------------------
// ------------------------------------------------------------------------------
// ------------------------------------------------------------------------------
// ------------------------------------------------------------------------------
// ------------------------------------------------------------------------------
// ------------------------------------------------------------------------------
// ------------------------------------------------------------------------------
// ------- UNIT ---------
// ------------------------------------------------------------------------------
// ------------------------------------------------------------------------------
// ------------------------------------------------------------------------------
// ------------------------------------------------------------------------------
// ------------------------------------------------------------------------------
// ------------------------------------------------------------------------------
// ------------------------------------------------------------------------------
// ------------------------------------------------------------------------------
// ------------------------------------------------------------------------------



vector<Coins*> Unit::getDroppableCoins()
{
    return vector<Coins*>{&goldInvested};
}
coinsInt Unit::getCost() const
{
    throw runtime_error("getCost() has not been defined for " + getTypename() + ".\n");
}
uint16_t Unit::getMaxHealth() const
{
    throw runtime_error("getMaxHeatlh() has not been defined for " + getTypename() + ".\n");
}
void Unit::cmdStop()
{
    throw runtime_error("cmdStop() has not been defined for " + getTypename() + ".\n");
}

Unit::Unit() : Entity() {} // this will throw if called. Needed for virtual inheritance later but should never be called.

Unit::Unit(uint8_t ownerId, coinsInt totalCost, uint16_t healthAssumingBuilt, vector2fp pos)
    : Entity(pos), healthAssumingBuilt(healthAssumingBuilt), ownerId(ownerId), goldInvested(totalCost), angle_view(0) {}
void Unit::packEntityAndUnitBasics(Netpack::Builder* to)
{
    packEntityBasics(to);

    to->packUint8_t(ownerId);
    to->packUint16_t(healthAssumingBuilt);
    goldInvested.pack(to);
}
Unit::Unit(Netpack::Consumer* from)
    : Entity(from),
      goldInvested((coinsInt)0), // will get overwritten with consume below
      angle_view(0)
{
    ownerId = from->consumeUint8_t();
    healthAssumingBuilt = from->consumeUint16_t();
    goldInvested = Coins(from);
}

coinsInt Unit::build(coinsInt attemptedAmount, Coins *fromCoins)
{
    return fromCoins->transferUpTo(attemptedAmount, &(this->goldInvested));
}
coinsInt Unit::unbuild(coinsInt attemptedAmount, Coins* toCoins)
{
    coinsInt amount = this->goldInvested.transferUpTo(attemptedAmount, toCoins);
    if (this->goldInvested.getInt() == 0)
    {
        die();
    }
    return amount;
}
bool Unit::completeBuildingInstantly(Coins* fromCoins)
{
    return fromCoins->tryTransfer(this->getCost() - this->goldInvested.getInt(), &this->goldInvested);
}

coinsInt Unit::getBuilt()
{
    return goldInvested.getInt();
}
fixed32 Unit::getBuiltRatio()
{
    return (fixed32)getBuilt() / getCost();
}
fixed32 Unit::getRange() const
{
    throw runtime_error("getRange has not been defined for '" + getTypename() + "'");
}
uint16_t Unit::getEffectiveHealth()
{
    if (getBuilt() == 0) return 1;

    // scales linearly from getMaxHealth() to 0 as unit builds
    uint16_t healthDeduction = static_cast<uint16_t>(getMaxHealth() * (1-getBuiltRatio()));
    if (healthDeduction >= healthAssumingBuilt)
    {
        cout << "LOGIC WARNING: Unit::getEffectiveHealth seems a bit wonky!" << endl;
        return 1;
    }

    return healthAssumingBuilt - healthDeduction;
}
bool Unit::isIdle() {
    throw runtime_error("isIdle has not been defined for '" + getTypename() + "'");
}

bool Unit::isActive()
{
    return
    (
        getBuilt() >= getCost() &&
        (!dead)
    );
}
sf::Color Unit::getTeamOrPrimaryColor()
{
    return getTeamColor();
}
sf::Color Unit::getTeamColor()
{
    if (ownerId == -1)
        return sf::Color(150, 150, 150);
    else if (auto regInfo = maybeRegInfo)
        return playerAddressToColor(this->getGameOrThrow()->playerIdToAddress(ownerId));
    else
        return sf::Color(150, 150, 150);
}
void Unit::iterateUnitBasics() {}
void Unit::takeHit(uint16_t damage)
{
    if (damage >= getEffectiveHealth())
    {
        healthAssumingBuilt = 0;
        dead = true;
    }
    else
    {
        healthAssumingBuilt -= damage;
    }
}
uint16_t Unit::getHealthAssumingBuilt()
{
    return healthAssumingBuilt;
}



// ------------------------------------------------------------------------------
// ------------------------------------------------------------------------------
// ------------------------------------------------------------------------------
// ------------------------------------------------------------------------------
// ------------------------------------------------------------------------------
// ------------------------------------------------------------------------------
// ------------------------------------------------------------------------------
// ------------------------------------------------------------------------------
// ------------------------------------------------------------------------------
// ------- BUILDING -----
// ------------------------------------------------------------------------------
// ------------------------------------------------------------------------------
// ------------------------------------------------------------------------------
// ------------------------------------------------------------------------------
// ------------------------------------------------------------------------------
// ------------------------------------------------------------------------------
// ------------------------------------------------------------------------------
// ------------------------------------------------------------------------------
// ------------------------------------------------------------------------------


Building::Building() {} // this will throw if called. Needed for virtual inheritance later but should never be called.

void Building::packBuildingBasics(Netpack::Builder* to) {}
Building::Building(Netpack::Consumer* from)
    : Unit(from) {}

void Building::iterateBuildingBasics()
{
    iterateUnitBasics();
}



// ------------------------------------------------------------------------------
// ------------------------------------------------------------------------------
// ------------------------------------------------------------------------------
// ------------------------------------------------------------------------------
// ------------------------------------------------------------------------------
// ------------------------------------------------------------------------------
// ------------------------------------------------------------------------------
// ------------------------------------------------------------------------------
// ------------------------------------------------------------------------------
// ------- MOBILEUNIT ---
// ------------------------------------------------------------------------------
// ------------------------------------------------------------------------------
// ------------------------------------------------------------------------------
// ------------------------------------------------------------------------------
// ------------------------------------------------------------------------------
// ------------------------------------------------------------------------------
// ------------------------------------------------------------------------------
// ------------------------------------------------------------------------------
// ------------------------------------------------------------------------------


MoveTargetInfo::MoveTargetInfo(Target target, fixed32 desiredRange, uint32_t closestDistanceFloorSquared)
    : target(target), desiredRange(desiredRange), closestDistanceFloorSquared(closestDistanceFloorSquared), frustration(0)
    {}
void MoveTargetInfo::pack(Netpack::Builder* to)
{
    target.pack(to);
    packFixed32(to, desiredRange);
    to->packUint32_t(closestDistanceFloorSquared);
    packFixed32(to, frustration);
}
MoveTargetInfo::MoveTargetInfo(Netpack::Consumer* from)
    : target((EntityRef)0)
{
    target = Target(from);
    desiredRange = consumeFixed32(from);
    closestDistanceFloorSquared = from->consumeUint32_t();
    frustration = consumeFixed32(from);
}

void packMoveTargetInfo(Netpack::Builder* to, MoveTargetInfo mti)
{
    mti.pack(to);
}
MoveTargetInfo consumeMoveTargetInfo(Netpack::Consumer* from)
{
    return MoveTargetInfo(from);
}

MobileUnit::MobileUnit() {} // this will throw if called. Needed for virtual inheritance later but should never be called.
void MobileUnit::packMobileUnitBasics(Netpack::Builder* to)
{
    to->packOptional<MoveTargetInfo>(maybeTargetInfo, packMoveTargetInfo);
    packVector2fp(to, desiredVelocity);
    packVector2fp(to, lastVelocity);
}
MobileUnit::MobileUnit(Netpack::Consumer* from)
    : Unit(from)
{
    maybeTargetInfo = from->consumeOptional(consumeMoveTargetInfo);
    desiredVelocity = consumeVector2fp(from);
    lastVelocity = consumeVector2fp(from);
}

fixed32 MobileUnit::getMaxSpeed() const
{
    throw runtime_error("getMaxSpeed has not been defined for '" + getTypename() + "'");
}
vector2fp MobileUnit::getDesiredVelocity() const
{
    return desiredVelocity;
}
vector2fp MobileUnit::getLastVelocity() const
{
    return lastVelocity;
}
void MobileUnit::onMoveCmd(vector2fp moveTo)
{
    throw runtime_error("onMoveCmd() has not been defined for '" + getTypename() + "'");
}

void MobileUnit::setMoveTarget(Target _target, fixed32 newRange)
{
    if (auto point = _target.getPointUnlessTargetDeleted(*this->getGameOrThrow()))
    {
        uint32_t currentRangeSquared = (this->getPos() - *point).getFloorMagnitudeSquared();
        maybeTargetInfo = {MoveTargetInfo(_target, newRange, currentRangeSquared)};
    }
    else
    {
        cout << "Trying to set a move target for a deleted unit! Just not gonna do that." << endl;
    }
}
void MobileUnit::clearMoveTarget()
{
    maybeTargetInfo = {};
}
void MobileUnit::mobileUnitStop()
{
    clearMoveTarget();
}
bool MobileUnit::mobileUnitIsIdle()
{
    if (auto targetInfo = maybeTargetInfo)
    {
        if (optional<vector2fp> p = targetInfo->target.getPointUnlessTargetDeleted(*getGameOrThrow()))
        {
            vector2fp toPoint = *p - getPos();
            fixed32 distanceLeft = toPoint.getRoughMagnitude() - targetInfo->desiredRange;
            return (distanceLeft <= EPSILON);
        }
        return true;
    }
    return true;
}
optional<Target> MobileUnit::getMaybeMoveTarget()
{
    if (maybeTargetInfo)
        return maybeTargetInfo->target;
    else
        return {};
}
optional<MoveTargetInfo> MobileUnit::getMaybeMoveTargetInfo()
{
    return maybeTargetInfo;
}

void MobileUnit::moveWithVelocityAndUpdateCell(vector2fp velocity)
{
    lastVelocity = velocity;
    if (velocity == vector2fp::zero)
    {
        return;
    }
    setPosAndUpdateCell(getPos() + velocity);
}
void MobileUnit::tryMoveTowardPoint(vector2fp to, fixed32 range)
{
    vector2fp toPoint = to - getPos();
    fixed32 distanceLeft = toPoint.getRoughMagnitude() - range;
    if (distanceLeft <= EPSILON)
    {
        return;
    }

    vector2fp unitDir = toPoint.normalized();
    angle_view = static_cast<float>(unitDir.getAngle());

    if (distanceLeft <= getMaxSpeed())
    {
        desiredVelocity = unitDir * min(getMaxSpeed(), toPoint.getRoughMagnitude());
        // the min in the above line ensures the unit takes a "full step" toward a possibly fleeing enemy
    }
    else
    {
        desiredVelocity = unitDir * getMaxSpeed();
    }
}
void MobileUnit::iterateMobileUnitBasics()
{
    desiredVelocity = vector2fp::zero;

    if (auto targetInfo = maybeTargetInfo)
    {
        if (optional<vector2fp> p = targetInfo->target.getPointUnlessTargetDeleted(*getGameOrThrow()))
        {
            uint32_t distanceFloorSquared = (getPos() - *p).getFloorMagnitudeSquared();
            // if we're "breaking a record" for closest to the point, set frustration to 0
            if (distanceFloorSquared <= targetInfo->closestDistanceFloorSquared)
            {
                maybeTargetInfo->closestDistanceFloorSquared = distanceFloorSquared;
                maybeTargetInfo->frustration = fixed32(0);
            }
            // otherwise, frustration mounts!
            else
            {
                maybeTargetInfo->frustration += MOBILEUNIT_FRUSTRATION_GROWTH_FACTOR;
            }

            // different satisfaction criteria depending on target type
            bool satisfied;
            if (targetInfo->target.type == Target::PointTarget)
            {
                // this factors in frustration, so the unit eventually gives up.
                // this is primarly to avoid frantic swarming when large numbers of units all go toward the same point.
                satisfied = (distanceFloorSquared <= floorSquareFixed(targetInfo->frustration));
            }
            else
            {
                // but if the target is an entity, we don't really want the unit to ever give up.
                satisfied = (distanceFloorSquared == 0); // note that this will happen whenever (getPos - *p) above is within the bounds (-1,-1) : (1, 1) due to the flooring.
            }

            if (satisfied)
            {
                clearMoveTarget();
            }
            else
            {
                tryMoveTowardPoint(*p, targetInfo->desiredRange);
            }
        }
        else
            clearMoveTarget();
    }
    iterateUnitBasics();
}
void MobileUnit::cmdMove(vector2fp pointTarget)
{
    setMoveTarget(Target(pointTarget), fixed32(0));
    onMoveCmd(pointTarget);
}



// ------------------------------------------------------------------------------
// ------------------------------------------------------------------------------
// ------------------------------------------------------------------------------
// ------------------------------------------------------------------------------
// ------------------------------------------------------------------------------
// ------------------------------------------------------------------------------
// ------------------------------------------------------------------------------
// ------------------------------------------------------------------------------
// ------------------------------------------------------------------------------
// ------- BEACON -------
// ------------------------------------------------------------------------------
// ------------------------------------------------------------------------------
// ------------------------------------------------------------------------------
// ------------------------------------------------------------------------------
// ------------------------------------------------------------------------------
// ------------------------------------------------------------------------------
// ------------------------------------------------------------------------------
// ------------------------------------------------------------------------------
// ------------------------------------------------------------------------------



fixed32 Beacon::getRadius() const { return BEACON_RADIUS; }
uint8_t Beacon::typechar() const { return BEACON_TYPECHAR; }
string Beacon::getTypename() const { return "Beacon"; }
coinsInt Beacon::getCost() const { return GATEWAY_COST; }
uint16_t Beacon::getMaxHealth() const { return GATEWAY_HEALTH; }

Beacon::Beacon(uint8_t ownerId, vector2fp pos, State state)
    : Unit(ownerId, GATEWAY_COST, GATEWAY_HEALTH, pos)
    , state(state)
{}
void Beacon::pack(Netpack::Builder* to)
{
    packEntityAndUnitBasics(to);
    packBuildingBasics(to);

    to->packEnum(state);
}
Beacon::Beacon(Netpack::Consumer* from)
    : Unit(from)
    , Building(from)
{
    state = from->consumeEnum<Beacon::State>();
}

void Beacon::iterate()
{
    Game *game = this->getGameOrThrow();

    switch (state)
    {
        case Spawning:
        {
            build(BEACON_BUILD_RATE, &getGameOrThrow()->players[ownerId].credit);

            if (isActive())
            {
                // replace self with a Gateway
                boost::shared_ptr<Gateway> gateway(new Gateway(this->ownerId, this->getPos()));
                gateway->completeBuildingInstantly(&this->goldInvested);
                this->die();
                game->registerNewEntityIgnoringCollision(gateway);
            }
        }
        break;
        case Despawning:
        {
            unbuild(BEACON_BUILD_RATE, &getGameOrThrow()->players[ownerId].credit);

            if (this->getBuilt() == 0)
            {
                die();
            }
        }
        break;
    }
}

void Beacon::cmdStop() {}



// ------------------------------------------------------------------------------
// ------------------------------------------------------------------------------
// ------------------------------------------------------------------------------
// ------------------------------------------------------------------------------
// ------------------------------------------------------------------------------
// ------------------------------------------------------------------------------
// ------------------------------------------------------------------------------
// ------------------------------------------------------------------------------
// ------------------------------------------------------------------------------
// ------- GATEWAY ------
// ------------------------------------------------------------------------------
// ------------------------------------------------------------------------------
// ------------------------------------------------------------------------------
// ------------------------------------------------------------------------------
// ------------------------------------------------------------------------------
// ------------------------------------------------------------------------------
// ------------------------------------------------------------------------------
// ------------------------------------------------------------------------------
// ------------------------------------------------------------------------------



fixed32 Gateway::getRadius() const {return GATEWAY_RADIUS;}
uint8_t Gateway::typechar() const { return GATEWAY_TYPECHAR; }
string Gateway::getTypename() const { return "Gateway"; }
coinsInt Gateway::getCost() const { return GATEWAY_COST; }
uint16_t Gateway::getMaxHealth() const { return GATEWAY_HEALTH; }

bool Gateway::isIdle() {
    return buildTargetQueue.size() == 0 && scuttleTargetQueue.size() == 0;
}
void Gateway::cmdStop()
{
    // press stop twice to stop all activity if both building and scuttling
    if (buildTargetQueue.size() > 0)
    {
        buildTargetQueue.clear();
    }
    else
    {
        scuttleTargetQueue.clear();
    }
}
void Gateway::removeFromQueues(EntityRef entityRef)
{
    for (unsigned int i=0; i<buildTargetQueue.size(); i++)
    {
        if (buildTargetQueue[i] == entityRef)
        {
            buildTargetQueue.erase(buildTargetQueue.begin() + i);
            i --;
        }
    }
    for (unsigned int i=0; i<scuttleTargetQueue.size(); i++)
    {
        if (scuttleTargetQueue[i] == entityRef)
        {
            buildTargetQueue.erase(buildTargetQueue.begin() + i);
            i --;
        }
    }
}

void Gateway::cmdBuildUnit(uint8_t unitTypechar)
{
    if (buildTargetQueue.size() >= 255)
        return;
    
    if (this->isActive())
    {
        // determine position of new unit. Hacky - should replace very soon!
        fixed32 angle = getGameOrThrow()->frame / fixed32(10);
        vector2fp newUnitPos = this->getPos() + composeVector2fp(angle, GATEWAY_RANGE / 2);
        boost::shared_ptr<Unit> littleBabyUnitAwwwwSoCute;
        switch (unitTypechar)
        {
            case PRIME_TYPECHAR:
                littleBabyUnitAwwwwSoCute = boost::shared_ptr<Prime>(new Prime(this->ownerId, newUnitPos));
                break;
            case FIGHTER_TYPECHAR:
                littleBabyUnitAwwwwSoCute = boost::shared_ptr<Fighter>(new Fighter(this->ownerId, newUnitPos));
                break;
            default:
                cout << "Gateway doesn't know how to build that unit..." << endl;
                break;
        }
        if (littleBabyUnitAwwwwSoCute)
        {
            getGameOrThrow()->registerNewEntityIgnoringCollision(littleBabyUnitAwwwwSoCute);
            this->buildTargetQueue.push_back(littleBabyUnitAwwwwSoCute->getRefOrThrow());
        }
    }
}
void Gateway::cmdDepositTo(Target target)
{
    if (buildTargetQueue.size() >= 255)
        return;
    
    if (this->isActive())
    {
        // if target is a point, check range and create goldPile
        if (auto point = target.castToPoint())
        {
            if ((*point - this->getPos()).getFloorMagnitudeSquared() > GATEWAY_RANGE_FLOORSQUARED)
            {
                return;
            }
            boost::shared_ptr<GoldPile> goldpile(new GoldPile(*point));
            getGameOrThrow()->registerNewEntityIgnoringCollision(goldpile);
            buildTargetQueue.push_back(goldpile->getRefOrThrow());
        }
        else if (auto entityRef = target.castToEntityRef())
        {
            buildTargetQueue.push_back(*entityRef);
        }
        else
        {
            cout << "Gateway can't cast that Target to a point or entity during cmdDepositTo" << endl;
        }
    }
}
void Gateway::cmdScuttle(EntityRef targetRef)
{
    if (scuttleTargetQueue.size() >= 255)
        return;
    
    Game *game = this->getGameOrThrow();

    if (this->isActive())
    {
        if (targetRef == this->getRefOrThrow())
        {
            // replace self with a despawning Beacon
            boost::shared_ptr<Unit> beacon(new Beacon(this->ownerId, this->getPos(), Beacon::Despawning));
            beacon->completeBuildingInstantly(&this->goldInvested);
            this->die();
            game->registerNewEntityIgnoringCollision(beacon);
        }
        else
        {
            if (auto entity = maybeEntityRefToPtrOrNull(*getGameOrThrow(), targetRef))
            {
                // first, make sure it's not already in either queue
                // the main point of doing this is to make sure we don't end up both scuttling and building the unit at the same time
                this->removeFromQueues(targetRef);

                if (auto unit = boost::dynamic_pointer_cast<Unit, Entity>(entity))
                {
                    if (getAllianceType(this->ownerId, unit) == Owned)
                    {
                        if ((this->getPos() - unit->getPos()).getFloorMagnitudeSquared() > GATEWAY_RANGE_FLOORSQUARED)
                        {
                            if (auto mobileUnit = boost::dynamic_pointer_cast<MobileUnit, Unit>(unit))
                            {
                                mobileUnit->setMoveTarget(this->getRefOrThrow(), GATEWAY_RANGE);
                                scuttleTargetQueue.push_back(targetRef);
                            }
                            else
                            {
                                // It's a building and it's out of range; ignore
                            }
                        }
                        else
                        {
                            scuttleTargetQueue.push_back(targetRef);
                        }
                    }
                    else
                    {
                        // Not owned by player; just ignore
                    }
                }
                else if (auto goldpile = boost::dynamic_pointer_cast<GoldPile, Entity>(entity))
                {
                    if ((this->getPos() - goldpile->getPos()).getFloorMagnitudeSquared() <= GATEWAY_RANGE_FLOORSQUARED)
                    {
                        scuttleTargetQueue.push_back(goldpile->getRefOrThrow());
                    }
                }
                else
                {
                    cout << "Somehow I can't cast that Entity into a GoldPile or Unit" << endl;
                }
            }
        }
    }
}
void Gateway::cmdStopScuttle(EntityRef targetRef)
{
    for (unsigned int i=0; i<scuttleTargetQueue.size(); i++)
    {
        if (scuttleTargetQueue[i] == targetRef)
        {
            scuttleTargetQueue.erase(scuttleTargetQueue.begin() + i);
            i --;
        }
    }
}

coinsInt Gateway::buildQueueWeight()
{
    Game *game = getGameOrThrow();

    coinsInt totalUnbuilt(0);

    for (unsigned int i=0; i<buildTargetQueue.size(); i++)
    {
        if (auto entity = game->entities[buildTargetQueue[i]])
        {
            if (auto goldpile = boost::dynamic_pointer_cast<GoldPile, Entity>(entity))
            {
                // since we put goldpiles behind everything regularly, we can count this as zero build queue weight
            }
            else if (auto unit = boost::dynamic_pointer_cast<Unit, Entity>(entity))
            {
                totalUnbuilt += unit->getCost() - unit->getBuilt();
            }
        }
    }

    return totalUnbuilt;
}
coinsInt Gateway::scuttleQueueWeight()
{
    Game *game = getGameOrThrow();

    coinsInt totalLeftToScuttle(0);

    for (unsigned int i=0; i<scuttleTargetQueue.size(); i++)
    {
        if (auto entity = game->entities[scuttleTargetQueue[i]])
        {
            if (auto goldpile = boost::dynamic_pointer_cast<GoldPile, Entity>(entity))
            {
                totalLeftToScuttle += goldpile->gold.getInt();
            }
            else if (auto unit = boost::dynamic_pointer_cast<Unit, Entity>(entity))
            {
                totalLeftToScuttle += unit->getBuilt();
            }
        }
    }

    return totalLeftToScuttle;
}

Gateway::Gateway(uint8_t ownerId, vector2fp pos)
    : Unit(ownerId, GATEWAY_COST, GATEWAY_HEALTH, pos),
      building_view(false), scuttling_view(false)
{}
void Gateway::pack(Netpack::Builder* to)
{
    packEntityAndUnitBasics(to);
    packBuildingBasics(to);

    if (buildTargetQueue.size() > 255)
        cout << "WARNING! buildTargetQueue.size() is greater than 255; pack will fail!" << endl;
    
    to->packUint8_t(buildTargetQueue.size());
    for (unsigned int i=0; i<buildTargetQueue.size(); i++)
    {
        packEntityRef(to, buildTargetQueue[i]);
    }

    if (scuttleTargetQueue.size() > 255)
        cout << "WARNING! scuttleTargetQueue.size() is greater than 255; pack will fail!" << endl;
    
    to->packUint8_t(scuttleTargetQueue.size());
    for (unsigned int i=0; i<scuttleTargetQueue.size(); i++)
    {
        packEntityRef(to, scuttleTargetQueue[i]);
    }
}
Gateway::Gateway(Netpack::Consumer* from)
    : Unit(from)
    , Building(from)
{
    auto buildTargetQueueSize = from->consumeUint8_t();
    buildTargetQueue.reserve(buildTargetQueueSize);
    for (unsigned int i=0; i<buildTargetQueueSize; i++)
    {
        EntityRef ref = consumeEntityRef(from);
        buildTargetQueue.push_back(ref);
    }

    auto scuttleTargetQueueSize = from->consumeUint8_t();
    scuttleTargetQueue.reserve(scuttleTargetQueueSize);
    for (unsigned int i=0; i<scuttleTargetQueueSize; i++)
    {
        EntityRef ref = consumeEntityRef(from);
        scuttleTargetQueue.push_back(ref);
    }
}

void Gateway::iterate()
{
    Game *game = getGameOrThrow();

    pushing_view = building_view = pulling_view = scuttling_view = false; // until proven otherwise

    // maybe there's something to scuttle
    if (scuttleTargetQueue.size() > 0)
    {
        // first let's iterate through and remove out of range units IF they're not both mobile and moving to the GW
        // we'll also ensure the first in-range entity is at the head of the queue
        bool foundInRangeUnit = false;
        for (unsigned int i=0; i<scuttleTargetQueue.size(); i++)
        {
            if (scuttleTargetQueue.size() == 0) break;

            if (auto entity = game->entities[scuttleTargetQueue[i]])
            {
                // Is the unit out of range?
                if ((this->getPos() - entity->getPos()).getFloorMagnitudeSquared() > GATEWAY_RANGE_FLOORSQUARED)
                {
                    // is it a mobile unit?
                    if (auto mobileUnit = boost::dynamic_pointer_cast<MobileUnit, Entity>(entity))
                    {
                        // if it's not on its way, boot from list
                        if ((!(mobileUnit->getMaybeMoveTarget())) || *(mobileUnit->getMaybeMoveTarget()->castToEntityRef()) != this->getRefOrThrow())
                        {
                            scuttleTargetQueue.erase(scuttleTargetQueue.begin() + i);
                            i --;
                            continue;
                        }
                    }
                    else // not a mobile unit and out of range; boot from list and continue
                    {
                        scuttleTargetQueue.erase(scuttleTargetQueue.begin() + i);
                        i --;
                        continue;
                    }
                }
                else // entity is in range. If this is the first such one we've found, make sure it's at the head of the queue.
                {
                    if (!foundInRangeUnit)
                    { // we only want to do this once
                        foundInRangeUnit = true;

                        if (i != 0)
                        {
                            // entity is not at front, so let's fix this with an erase and insert
                            // (note: std::rotate would be more efficient here, but for simplicity for now...)

                            EntityRef refToMove = scuttleTargetQueue[i];
                            scuttleTargetQueue.erase(scuttleTargetQueue.begin() + i);
                            scuttleTargetQueue.insert(scuttleTargetQueue.begin(), refToMove);
                        }
                    }
                }
            }
            else // entity was null; remove from list
            {
                scuttleTargetQueue.erase(scuttleTargetQueue.begin() + i);
                i --;
            }
        }

        // okay, we now know that
        //   - all entities in the queue are either in range or moving toward the GW
        //   - all entities in the queue are non-null
        //   - if any entities are in-range, the first one is at the head of the queue
        
        // make sure we still have something in the queue
        if (scuttleTargetQueue.size() > 0)
        {
            auto entity = game->entities[scuttleTargetQueue[0]];

            // check it's in range
            if ((this->getPos() - entity->getPos()).getFloorMagnitudeSquared() <= GATEWAY_RANGE_FLOORSQUARED)
            {
                // we have something to scuttle!
                coinsInt amountPulled(0);
                if (auto goldPile = boost::dynamic_pointer_cast<GoldPile, Entity>(entity))
                {
                    amountPulled = goldPile->gold.transferUpTo(GOLD_TRANSFER_RATE, &game->players[this->ownerId].credit);
                    scuttling_view = true;
                }
                else if (auto unit = boost::dynamic_pointer_cast<Unit, Entity>(entity))
                {
                    bool pulledFromPrimeHeldGold = false;
                    if (auto prime = boost::dynamic_pointer_cast<Prime, Unit>(unit))
                    {
                        if (prime->heldGold.getInt() > 0)
                        {
                            amountPulled = prime->heldGold.transferUpTo(GOLD_TRANSFER_RATE, &game->players[this->ownerId].credit);
                            pulledFromPrimeHeldGold = true;
                        }
                    }
                    if (!pulledFromPrimeHeldGold)
                    {
                        amountPulled = unit->unbuild(GOLD_TRANSFER_RATE, &game->players[this->ownerId].credit);
                    }
                    scuttling_view = true;
                }

                if (amountPulled > 0)
                {
                    pulling_view = true;
                }
                else
                {
                    // can't scuttle no more, remove from queue
                    scuttleTargetQueue.erase(scuttleTargetQueue.begin());
                }   
            }
        }
    }

    // if we have money, maybe there's something to build?
    if (game->players[this->ownerId].credit.getInt() > 0)
    {
        if (buildTargetQueue.size() > 0)
        {
            if (auto entity = game->entities[buildTargetQueue[0]])
            {
                // let's make sure no goldpiles are "blocking" any other finite jobs in the queue
                if (auto firstGoldpile = boost::dynamic_pointer_cast<GoldPile, Entity>(entity))
                {
                    // for now, hacky but simple: if there are any non-gold items in the queue, then move this gp to the end
                    
                    bool nonGoldpileFound = false;
                    for (unsigned int i=0; i<buildTargetQueue.size(); i++)
                    {
                        if (game->entities[buildTargetQueue[i]]->typechar() != GOLDPILE_TYPECHAR)
                        {
                            nonGoldpileFound = true;
                            break;
                        }
                    }
                    
                    if (nonGoldpileFound)
                    {
                        // would be better to use std::rotate, but for now:
                        buildTargetQueue.push_back(buildTargetQueue[0]);
                        buildTargetQueue.erase(buildTargetQueue.begin());
                    }
                }

                // check range
                if ((this->getPos() - entity->getPos()).getFloorMagnitudeSquared() <= GATEWAY_RANGE_FLOORSQUARED)
                {
                    Coins* maybeCoinsToDepositTo = NULL;
                    boost::shared_ptr<Unit> maybeBuildingUnit;
                    if (auto goldpile = boost::dynamic_pointer_cast<GoldPile, Entity>(entity))
                    {
                        maybeCoinsToDepositTo = &goldpile->gold;
                    }
                    else if (auto unit = boost::dynamic_pointer_cast<Unit, Entity>(entity))
                    {
                        if (unit->getBuiltRatio() < fixed32(1))
                        {
                            maybeCoinsToDepositTo = &unit->goldInvested;
                            maybeBuildingUnit = unit;
                            building_view = true;
                        }
                        else if (auto prime = boost::dynamic_pointer_cast<Prime, Unit>(unit))
                        {
                            maybeCoinsToDepositTo = &prime->heldGold;
                        }
                    }

                    if (maybeCoinsToDepositTo)
                    {
                        coinsInt amountDeposited = game->players[this->ownerId].credit.transferUpTo(GOLD_TRANSFER_RATE, maybeCoinsToDepositTo);
                        if (maybeBuildingUnit && maybeBuildingUnit->getBuiltRatio() == fixed32(1))
                        {
                            buildTargetQueue.erase(buildTargetQueue.begin());
                        }
                        if (amountDeposited == 0)
                        {
                            buildTargetQueue.erase(buildTargetQueue.begin());
                        }
                        if (amountDeposited > 0)
                        {
                            pushing_view = true;
                        }
                    }
                    else
                    {
                        buildTargetQueue.erase(buildTargetQueue.begin());
                    }
                }
                else // entity out of range, remove from list
                {
                    buildTargetQueue.erase(buildTargetQueue.begin());
                }
            }
            else // entity was dead or null or something; remove from list
            {
                buildTargetQueue.erase(buildTargetQueue.begin());
            }
        }
    }
}



// ------------------------------------------------------------------------------
// ------------------------------------------------------------------------------
// ------------------------------------------------------------------------------
// ------------------------------------------------------------------------------
// ------------------------------------------------------------------------------
// ------------------------------------------------------------------------------
// ------------------------------------------------------------------------------
// ------------------------------------------------------------------------------
// ------------------------------------------------------------------------------
// ------- PRIME --------
// ------------------------------------------------------------------------------
// ------------------------------------------------------------------------------
// ------------------------------------------------------------------------------
// ------------------------------------------------------------------------------
// ------------------------------------------------------------------------------
// ------------------------------------------------------------------------------
// ------------------------------------------------------------------------------
// ------------------------------------------------------------------------------
// ------------------------------------------------------------------------------



fixed32 Prime::getRadius() const { return PRIME_RADIUS; }
fixed32 Prime::getMaxSpeed() const { return PRIME_SPEED; }
fixed32 Prime::getRange() const { return PRIME_TRANSFER_RANGE; }
coinsInt Prime::getCost() const { return PRIME_COST; }
uint16_t Prime::getMaxHealth() const { return PRIME_HEALTH; }

uint8_t Prime::typechar() const { return PRIME_TYPECHAR; }
string Prime::getTypename() const { return "Prime"; }

Prime::Prime(uint8_t ownerId, vector2fp pos)
    : Unit(ownerId, PRIME_COST, PRIME_HEALTH, pos),
      heldGold(PRIME_MAX_GOLD_HELD),
      goldTransferState_view(NoGoldTransfer)
{}
void Prime::pack(Netpack::Builder* to)
{
    packEntityAndUnitBasics(to);
    packMobileUnitBasics(to);

    heldGold.pack(to);
}
Prime::Prime(Netpack::Consumer* from)
    : Unit(from)
    , MobileUnit(from),
    heldGold(PRIME_MAX_GOLD_HELD)
{
    heldGold = Coins(from);
}

void Prime::cmdDeposit(Target _target)
{
    if (auto point = _target.castToPoint())
    {
        this->fundsDest = _target;
        setMoveTarget(_target, PRIME_TRANSFER_RANGE);
    }
    else if (auto entity = _target.castToEntityPtr(*getGameOrThrow()))
    {
        if (auto goldpile = boost::dynamic_pointer_cast<GoldPile, Entity>(entity))
        {
            this->fundsDest = _target;
            setMoveTarget(_target, PRIME_TRANSFER_RANGE);
        }
        else if (auto unit = boost::dynamic_pointer_cast<Unit, Entity>(entity))
        {
            if (unit->getBuiltRatio() < fixed32(1))
            {
                this->buildTargetQueue.push_back(unit->getRefOrThrow());
                setMoveTarget(_target, PRIME_TRANSFER_RANGE);
            }
            else
            {
                if (unit->typechar() == GATEWAY_TYPECHAR || unit->typechar() == PRIME_TYPECHAR)
                {
                    this->fundsDest = _target;
                    setMoveTarget(_target, PRIME_TRANSFER_RANGE);
                }
                else
                {
                    // This is a built unit that is not a Gateway or Prime. Ignore.
                }
            }
        }
    }
}

void Prime::cmdFetch(Target _target)
{
    if (auto point = _target.castToPoint())
    {
        this->scavengeTargetQueue.push_back(_target);
    }
    else if (auto entity = _target.castToEntityPtr(*getGameOrThrow()))
    {
        if (auto goldpile = boost::dynamic_pointer_cast<GoldPile, Entity>(entity))
        {
            this->scavengeTargetQueue.push_back(_target);
        }
    }
}

void Prime::cmdStop()
{
    mobileUnitStop();

    buildTargetQueue.clear();
    fundsDest = {};
    scavengeTargetQueue.clear();
    fundsSource = {};
}

bool Prime::isIdle()
{
    bool hasSourceAndDest = (bool(getMaybeFetchTarget() && bool(getMaybeDepositTarget())));

    return mobileUnitIsIdle() && !hasSourceAndDest;
}

void Prime::onMoveCmd(vector2fp moveTo)
{
    buildTargetQueue.clear();
    fundsDest = {};
    scavengeTargetQueue.clear();
    fundsSource = {};
}

optional<Target> Prime::getMaybeFetchTarget()
{
    if (scavengeTargetQueue.size() > 0)
    {
        return scavengeTargetQueue[0];
    }
    else if (fundsSource)
    {
        return Target(*fundsSource);
    }
    else
    {
        return {};
    }
}
optional<Target> Prime::getMaybeDepositTarget()
{
    if (buildTargetQueue.size() > 0)
    {
        return Target(buildTargetQueue[0]);
    }
    else
    {
        return fundsDest;
    }
}

void Prime::validateTargets()
{
    Game* game = getGameOrThrow();
    // Remove invalid targets, as well as build orders that are completed

    if (fundsSource)
    {
        if (! game->entities[*fundsSource])
        {
            // entity died. Clear it
            fundsSource = {};
        }
    }
    if (fundsDest)
    {
        if (! fundsDest->getPointUnlessTargetDeleted(*getGameOrThrow()))
        {
            // entity died. Clear it
            fundsDest = {};
        }
    }

    for (unsigned int i=0; i<buildTargetQueue.size(); i++)
    {
        if (auto entity = game->entities[buildTargetQueue[i]])
        {
            if (auto unit = boost::dynamic_pointer_cast<Unit, Entity>(entity))
            {
                if (unit->getBuiltRatio() == fixed32(1))
                {
                    buildTargetQueue.erase(buildTargetQueue.begin() + i);
                    i --;
                    continue;
                }
            }
            else
            {
                cout << "Got a gold pile in a Prime's buildTargetQueue. That seems kinda weird! Removing from queue." << endl;
                buildTargetQueue.erase(buildTargetQueue.begin() + i);
                i --;
                continue;
            }
        }
        else
        {
            // entity died
            buildTargetQueue.erase(buildTargetQueue.begin() + i);
            i --;
            continue;
        }
    }
    for (unsigned int i=0; i<scavengeTargetQueue.size(); i++)
    {
        if (! scavengeTargetQueue[i].isStillValid(*game))
        {
            // entity died
            scavengeTargetQueue.erase(scavengeTargetQueue.begin() + i);
            i --;
            continue;
        }
    }
}

void Prime::tryTransferAndMaybeMoveOn()
{
    Game* game = getGameOrThrow();

    // Transfer (possibly to and from, both at once), and handle removing targets or switching moveTarget
    optional<Target> fetchTarget = getMaybeFetchTarget();
    optional<Target> depositTarget = getMaybeDepositTarget();

    // First, pull from the fetchTarget if possible
    if (fetchTarget && heldGold.getSpaceLeft() > 0)
    {
        if (auto fetchTargetPoint = fetchTarget->getPointUnlessTargetDeleted(*game))
        {
            if ((this->getPos() - *fetchTargetPoint).getFloorMagnitudeSquared() <= PRIME_TRANSFER_RANGE_FLOORSQUARED)
            {
                if (auto entity = fetchTarget->castToEntityPtr(*game))
                {
                    // make sure we're still allowed to do this
                    if (getAllianceType(this->ownerId, entity) != Foreign)
                    {
                        coinsInt amountPulled(0);
                        bool scuttling = false;

                        if (auto goldpile = boost::dynamic_pointer_cast<GoldPile, Entity>(entity))
                        {
                            amountPulled = goldpile->gold.transferUpTo(GOLD_TRANSFER_RATE, &this->heldGold);
                        }
                        else if (auto unit = boost::dynamic_pointer_cast<Unit, Entity>(entity))
                        {
                            bool pulledFromPrimeHeldGold = false;
                            if (auto prime = boost::dynamic_pointer_cast<Prime, Unit>(unit))
                            {
                                if (prime->heldGold.getInt() > 0)
                                {
                                    amountPulled = prime->heldGold.transferUpTo(GOLD_TRANSFER_RATE, &this->heldGold);
                                    pulledFromPrimeHeldGold = true;
                                }
                            }
                            if (!pulledFromPrimeHeldGold)
                            {
                                amountPulled = unit->unbuild(GOLD_TRANSFER_RATE, &this->heldGold);
                                scuttling = true;
                            }
                        }

                        if (amountPulled > 0)
                        {
                            goldTransferState_view = scuttling ? ScuttlingSomething : Pulling;
                        }
                    }
                }
                else
                {
                    // we've arrived at the fetch target point. Remove it from the list.
                    if (scavengeTargetQueue.size() > 0 && *fetchTarget == scavengeTargetQueue[0])
                    {
                        scavengeTargetQueue.erase(scavengeTargetQueue.begin());
                    }
                    else
                    {
                        cout << "Warning: logic error related to Prime scavenge queue." << endl;
                    }
                }
            }
        }
        else
        {
            cout << "Logic error. Can't cast fetchTarget to point, but this should have been validated earlier in Prime::iterate..." << endl;
        }

        if (heldGold.getSpaceLeft() == 0)
        {
            // if we're here, the Prime just filled up. If we're not in range of the depositTarget, start moving there.
            if (depositTarget)
            {
                if (auto depositTargetPoint = depositTarget->getPointUnlessTargetDeleted(*game))
                {
                    if ((this->getPos() - *depositTargetPoint).getFloorMagnitudeSquared() > PRIME_TRANSFER_RANGE_FLOORSQUARED)
                    {
                        setMoveTarget(*depositTarget, PRIME_TRANSFER_RANGE);
                    }
                }
                else
                {
                    cout << "Logic error. Can't cast depositTarget to point, but this should have been validated earlier in Prime::iterate..." << endl;
                }
            }
        }
    }

    // Now deposit to something if we can
    if (depositTarget && heldGold.getInt() > 0)
    {
        if (auto depositTargetPoint = depositTarget->getPointUnlessTargetDeleted(*game))
        {
            if ((this->getPos() - *depositTargetPoint).getFloorMagnitudeSquared() <= PRIME_TRANSFER_RANGE_FLOORSQUARED)
            {
                if (auto entity = depositTarget->castToEntityPtr(*game))
                {
                    coinsInt amountPushed(0);
                    bool building = false;

                    if (auto goldpile = boost::dynamic_pointer_cast<GoldPile, Entity>(entity))
                    {
                        amountPushed = this->heldGold.transferUpTo(GOLD_TRANSFER_RATE, &goldpile->gold);
                    }
                    else if (auto unit = boost::dynamic_pointer_cast<Unit, Entity>(entity))
                    {
                        if (unit->getBuiltRatio() < fixed32(1))
                        {
                            amountPushed = unit->build(GOLD_TRANSFER_RATE, &this->heldGold);
                            building = true;
                        }
                        else
                        {
                            if (auto gateway = boost::dynamic_pointer_cast<Gateway, Unit>(unit))
                            {
                                // TODO
                            }
                            if (auto prime = boost::dynamic_pointer_cast<Prime, Unit>(unit))
                            {
                                amountPushed = this->heldGold.transferUpTo(GOLD_TRANSFER_RATE, &prime->heldGold);
                            }
                            else
                            {
                                cout << "Warning: somehow a Prime is trying to deposit to a built unit other than a GW or Prime." << endl;
                            }
                        }
                    }

                    if (amountPushed > 0)
                    {
                        goldTransferState_view = building ? BuildingSomething : Pushing;
                    }
                }
                else
                {
                    // we've arrived at the deposit target point. Remove it from the list.
                    if (buildTargetQueue.size() > 0 && depositTarget->castToEntityRef() == buildTargetQueue[0])
                    {
                        buildTargetQueue.erase(buildTargetQueue.begin());
                    }
                    else
                    {
                        cout << "Warning: logic error related to Prime build queue." << endl;
                    }
                }
            }
        }
    }
}

void Prime::iterate()
{
    validateTargets();

    goldTransferState_view = NoGoldTransfer; // until proven otherwise

    Game *game = getGameOrThrow();

    optional<Target> fetchTarget = getMaybeFetchTarget();
    optional<Target> depositTarget = getMaybeDepositTarget();

    // We have three main sources of state, with which to implement higher-level logic (through reading and modifying):
    //   fetchTarget (optional)
    //   depositTarget (optional)
    //   MobileUnit::moveTarget (optional)

    // moveTarget represents our mose immediate action: moving toward some target or picking up / depositing something once we're there.

    if (!mobileUnitIsIdle())
    {
        // if we're still moving somewhere, there's nothing for us to do.
    }
    // thus in the below branches, mobileUnitIsIdle() == true, meaning we aren't moving toward anything
    else if (!(fetchTarget || depositTarget))
    {
        // No targets, nothing to do.
    }
    // is the moveTarget the same as one of the targets?
    else if ((fetchTarget && fetchTarget == getMaybeMoveTarget()) || (depositTarget && depositTarget == getMaybeMoveTarget()))
    {
        tryTransferAndMaybeMoveOn();
    }
    else
    {
        // if we're here, that means:
        //   * we're not moving anywhere (mobileUnitIsIdle() == true)
        //   * we have one of (fetchTarget, depositTarget) set and valid
        //   * moveTarget is not equal to either (fetchTarget, depositTarget)
        // Thus, we need to set moveTarget to one of these targets.

        // do we have one target set, or both?
        if (fetchTarget && depositTarget) // both are set
        {
            // how do we determine whether to start fetching or depositing?

            // If the Prime is full or empty, easy choice
            if (heldGold.getInt() == 0)
            {
                setMoveTarget(*fetchTarget, PRIME_TRANSFER_RANGE);
            }
            else if (heldGold.getSpaceLeft() == 0)
            {
                setMoveTarget(*depositTarget, PRIME_TRANSFER_RANGE);
            }
            else
            {
                // We have some gold but aren't yet full

                // If we're already in range of one, just try to transfer

                auto fetchTargetPoint = fetchTarget->getPointUnlessTargetDeleted(*game);
                auto depositTargetPoint = depositTarget->getPointUnlessTargetDeleted(*game);
                // We don't need to check these, since at the beginning of Iterate we validated all Targets.
                // We know the target pointers are set, but not necessarily that any pointed-to entities are still alive.

                if (((getPos() - *fetchTargetPoint).getFloorMagnitudeSquared() <= PRIME_TRANSFER_RANGE_FLOORSQUARED)
                 || ((getPos() - *depositTargetPoint).getFloorMagnitudeSquared() <= PRIME_TRANSFER_RANGE_FLOORSQUARED)
                )
                {
                    tryTransferAndMaybeMoveOn();
                }
                
                else
                {
                    // not in range of either target.
                    // Make a decision based on distance left to travel.
                    fixed32 distanceLeftToFetchTarget = max(fixed32(0), (getPos() - *fetchTargetPoint).getRoughMagnitude() - PRIME_TRANSFER_RANGE);
                    fixed32 distanceLeftToDepositTarget = max(fixed32(0), (getPos() - *depositTargetPoint).getRoughMagnitude() - PRIME_TRANSFER_RANGE);

                    if (distanceLeftToFetchTarget < distanceLeftToDepositTarget)
                    {
                        setMoveTarget(*fetchTarget, PRIME_TRANSFER_RANGE);
                    }
                    else
                    {
                        setMoveTarget(*depositTarget, PRIME_TRANSFER_RANGE);
                    }
                }
            }
        }
        else // only one of (fetchTarget, depositTarget) is set
        {
            if (fetchTarget)
            {
                if (heldGold.getSpaceLeft() > 0)
                {
                    setMoveTarget(*fetchTarget, PRIME_TRANSFER_RANGE);
                }
            }
            else // depositTarget
            {
                if (heldGold.getInt() > 0)
                {
                    setMoveTarget(*depositTarget, PRIME_TRANSFER_RANGE);
                }
            }
        }
    }

    /// --- OLD CODE --- ///

    // // first we process behavior. This is upstream from state.
    // switch (behavior)
    // {
    //     case Basic:
    //         break;
    //     case Gather:
    //     {
    //         if (auto gatherTargetPos = maybeGatherTargetPos)
    //         {
    //             // the state indicates what part of the cycle the Prime is in:
    //                 // NotTransferring: moving toward the gatherTargetPos
    //                 // PickupGold: moving toward or picking up some gold it found
    //                 // PutodownGold: bringing Gold to gateway or depositing
                
    //             // our job here is to switch these states when necessary...

    //             // firstly, if heldGold is maxed and we're not already on a return trip, return gold to nearest gateway
    //             if (getHeldGoldRatio() == fixed32(1) && state != PutdownGold)
    //             {
    //                 setStateToReturnGoldOrResetBehavior();
    //             }

    //             switch (state)
    //             {
    //                 case NotTransferring:
    //                 {
    //                     // in theory this should already be set, but in some cases doesn't seem to be
    //                     setMoveTarget(*gatherTargetPos, fixed32(0));
                        
    //                     // scan for any GoldPile and choose the closest
    //                     boost::shared_ptr<GoldPile> bestTarget;
    //                     uint16_t bestTargetDistanceFloorSquared;

    //                     auto entitiesInSight = game->entitiesWithinCircle(getPos(), PRIME_SIGHT_RANGE);
    //                     for (unsigned int i=0; i<entitiesInSight.size(); i++)
    //                     {
    //                         if (auto goldPile = boost::dynamic_pointer_cast<GoldPile, Entity>(entitiesInSight[i]))
    //                         {
    //                             // First, ignore any goldPile already within range of a Gateway
    //                             if (goldPile->isWithinRangeOfActiveGatewayOwnedBy(this->ownerId))
    //                                 continue;
                                
    //                             uint16_t distanceFloorSquared = (this->getPos() - goldPile->getPos()).getFloorMagnitudeSquared();
    //                             if (!bestTarget || distanceFloorSquared < bestTargetDistanceFloorSquared)
    //                             {
    //                                 bestTarget = goldPile;
    //                                 bestTargetDistanceFloorSquared = distanceFloorSquared;
    //                             }
    //                         }
    //                     }

    //                     // if we found something, set moveTarget to it and switch state to PickupGold
    //                     if (bestTarget)
    //                     {
    //                         state = PickupGold;
    //                         setMoveTarget(bestTarget->getRefOrThrow(), PRIME_TRANSFER_RANGE);
    //                         break;
    //                     }
    //                     // otoh, if we've arrived at the target without finding Gold to pickup...
    //                     else if ((*gatherTargetPos - this->getPos()).getFloorMagnitudeSquared() == 0)
    //                     {
    //                         // if we have no Gold, return to basic/idle behavior
    //                         if (heldGold.getInt() == 0)
    //                         {
    //                             behavior = Basic;
    //                             state = NotTransferring;
    //                             maybeGatherTargetPos = {};
    //                             break;
    //                         }
    //                         // otherwise, make a return trip
    //                         else
    //                         {
    //                             setStateToReturnGoldOrResetBehavior();
    //                         }
    //                     }
    //                     else
    //                     {
    //                         // just to be explicit, in this case we haven't found any gold
    //                         // but we are still traveling to the destination
    //                         // do nothing.
    //                     }
    //                 }
    //                 break;
    //                 case PickupGold:
    //                 {
    //                     // note that if heldGoldRatio == 1, we will have caught this just before this switch block,
    //                     // so we can assume that heldGoldRatio < 1 for now, and focus on gathering more.

    //                     bool shouldMoveOn = true; // until proven otherwise

    //                     // here, we need to do something either when the Prime is full or when its current Target gets depleted
    //                     if (auto moveTarget = getMaybeMoveTarget())
    //                     {
    //                         if (auto goldPile = boost::dynamic_pointer_cast<GoldPile, Entity>(moveTarget->castToEntityPtr(*game)))
    //                         {
    //                             if (goldPile->gold.getInt() > 0)
    //                             {
    //                                 shouldMoveOn = false;
    //                             }
    //                         }
    //                     }

    //                     if (shouldMoveOn)
    //                     {
    //                         state = NotTransferring;
    //                         setMoveTarget(*gatherTargetPos, fixed32(0));
    //                         break;
    //                     }
                        
    //                 }
    //                 break;
    //                 case PutdownGold:
    //                 {
    //                     if (heldGold.getInt() == 0)
    //                     {
    //                         state = NotTransferring;
    //                         setMoveTarget(*gatherTargetPos, fixed32(0));
    //                         break;
    //                     }
    //                     // as long as dropoff point (in moveTarget) is still valid, just continue til heldGold == 0
    //                     bool stillMovingTowardDropoff = false; // until proven otherwise
    //                     if (auto moveTarget = getMaybeMoveTarget())
    //                     {
    //                         stillMovingTowardDropoff = true;
    //                     }

    //                     if (!stillMovingTowardDropoff)
    //                     {
    //                         // find another dropoff point
    //                         setStateToReturnGoldOrResetBehavior();
    //                     }
    //                 }
    //                 break;
    //                 default:
    //                 {
    //                     cout << "Prime is in an unexpected combo of behavior and state. Returning to basic behavior." << endl;
    //                     behavior = Basic;
    //                 }
    //                 break;
    //             }
    //         }
    //         else
    //         {
    //             cout << "Behavior is Gather, but there is no gatherPos..." << endl;
    //             behavior = Basic;
    //             state = NotTransferring;
    //             break;
    //         }
    //     }
    //         break;
    // }

    // goldTransferState_view = NoGoldTransfer;
    // switch (state)
    // {
    // case NotTransferring:
    //     break;
    // case PickupGold:
    //     if (auto target = getMaybeMoveTarget())
    //     {
    //         if (boost::shared_ptr<Entity> e = target->castToEntityPtr(*game))
    //         {
    //             if ((e->getPos() - this->getPos()).getFloorMagnitudeSquared() <= PRIME_TRANSFER_RANGE_FLOORSQUARED)
    //             {
    //                 optional<Coins*> coinsToPullFrom;
    //                 if (auto goldpile = boost::dynamic_pointer_cast<GoldPile, Entity>(e))
    //                 {
    //                     coinsToPullFrom = &goldpile->gold;
    //                 }
    //                 if (coinsToPullFrom)
    //                 {
    //                     coinsInt pickedUp = (*coinsToPullFrom)->transferUpTo(GOLD_TRANSFER_RATE, &(this->heldGold));
    //                     if (pickedUp == 0)
    //                         state = NotTransferring;
    //                     else
    //                         goldTransferState_view = ScuttlingSomething;
    //                 }
    //             }
    //         }
    //     }
    //     break;
    // case PutdownGold:
    //     if (auto target = getMaybeMoveTarget())
    //     if (optional<vector2fp> point = target->getPointUnlessTargetDeleted(*game))
    //     {
    //         // special case if target is an active Gateway
    //         if (auto gateway = boost::dynamic_pointer_cast<Gateway, Entity>(target->castToEntityPtr(*game)))
    //         {
    //             if (gateway->isActive() && (gateway->getPos() - this->getPos()).getRoughMagnitude() <= PRIME_TRANSFER_RANGE + GATEWAY_RANGE)
    //             {
    //                 bool goldPileFound = false;
    //                 for (unsigned int i=0; i<gateway->scuttleTargetQueue.size(); i++)
    //                 {
    //                     if (auto goldpile = boost::dynamic_pointer_cast<GoldPile, Entity>(game->entities[gateway->scuttleTargetQueue[i]]))
    //                     {
    //                         setMoveTarget(goldpile->getRefOrThrow(), PRIME_TRANSFER_RANGE);
    //                         goldPileFound = true;
    //                     }
    //                 }

    //                 if (!goldPileFound)
    //                 {
    //                     // create gold pile at (almost) max gateway range toward self
    //                     // note that this will happen even if Prime is inside Gateway range, putting the new gold pile farther from the GW..
    //                     // a bit weird, but anything more elegant-looking is too complicated to implement atm
    //                     vector2fp gwToPrime = (this->getPos() - gateway->getPos());
    //                     vector2fp gwToNewGoldpile = gwToPrime.normalized() * GATEWAY_RANGE * fixed32(0.99);
    //                     setMoveTarget(gateway->getPos() + gwToNewGoldpile, PRIME_TRANSFER_RANGE);
    //                     depositingToGateway = true;
    //                 }
    //             }
    //         }
    //         target = getMaybeMoveTarget();
    //         if ((*point - getPos()).getFloorMagnitudeSquared() <= PRIME_TRANSFER_RANGE_FLOORSQUARED)
    //         {
    //             optional<Coins*> coinsToPushTo;
    //             bool buildingSomething = false;
    //             if (auto entity = target->castToEntityPtr(*game))
    //             {
    //                 if (auto goldpile = boost::dynamic_pointer_cast<GoldPile, Entity>(entity))
    //                 {
    //                     coinsToPushTo = &goldpile->gold;
    //                 }
    //                 else if (auto unit = boost::dynamic_pointer_cast<Unit, Entity>(entity))
    //                 {
    //                     // first try to complete it if it's not yet built
    //                     if (unit->getBuiltRatio() < fixed32(1))
    //                     {
    //                         coinsToPushTo = &unit->goldInvested;
    //                         buildingSomething = true;
    //                     }
    //                     else if (auto prime = boost::dynamic_pointer_cast<Prime, Unit>(unit))
    //                     {
    //                         coinsToPushTo = &prime->heldGold;
    //                     }
    //                     else
    //                     {
    //                         // nothing else to do then.
    //                         state = NotTransferring;
    //                     }
    //                 }
    //                 else {
    //                     cout << "not sure how to execute a PutdownGold cmd for a unit other than a gateway or goldpile" << endl;
    //                     state = NotTransferring;
    //                 }
    //             }
    //             else
    //             {
    //                 // must create goldPile
    //                 boost::shared_ptr<GoldPile> gp(new GoldPile(*point));
    //                 game->registerNewEntityIgnoringCollision(gp);
    //                 coinsToPushTo = &gp->gold;
    //                 setMoveTarget(Target(gp->getRefOrThrow()), PRIME_TRANSFER_RANGE);

    //                 // if we're creating a new gold pile for a GW, add it to the GW's scuttle queue
    //                 if (depositingToGateway)
    //                 {
    //                     // shit, need to somehow know the GW... could do another search at this point maybe?
    //                     // maybe find all GWs in range, for that matter
    //                     auto nearbyGateways = filterForType<Gateway,Entity>(game->entitiesWithinCircle(gp->getPos(), GATEWAY_RANGE));
    //                     for (unsigned int i=0; i<nearbyGateways.size(); i++)
    //                     {
    //                         nearbyGateways[i]->scuttleTargetQueue.push_back(gp->getRefOrThrow());
    //                     }
    //                 }
    //             }

    //             if (coinsToPushTo)
    //             {
    //                 coinsInt amountPutDown = this->heldGold.transferUpTo(GOLD_TRANSFER_RATE, (*coinsToPushTo));
    //                 if (amountPutDown == 0 && buildingSomething)
    //                 {
    //                     state = NotTransferring;
    //                 }
    //                 if (amountPutDown != 0)
    //                 {
    //                     goldTransferState_view = buildingSomething ? BuildingSomething : Pushing;
    //                 }
    //             }
    //             else
    //             {
    //                 cout << "I'm confused about how to execute this putdownGold cmd." << endl;
    //                 state = NotTransferring;
    //             }
    //         }
    //     }
    //     break;
    // case Build:
    //     if (auto target = getMaybeMoveTarget())
    //     {
    //         if (auto targetPos = target->getPointUnlessTargetDeleted(*game))
    //         {
    //             if ((*targetPos - getPos()).getFloorMagnitudeSquared() <= PRIME_TRANSFER_RANGE_FLOORSQUARED)
    //             {
    //                 if (optional<vector2fp> point = target->castToPoint())
    //                 {
    //                     // create unit if typechar checks out and change target to new unit
    //                     boost::shared_ptr<Building> buildingToBuild;
    //                     switch (gonnabuildTypechar)
    //                     {
    //                         case GATEWAY_TYPECHAR:
    //                             buildingToBuild = boost::shared_ptr<Building>(new Gateway(this->ownerId, *point));
    //                             break;
    //                         case TURRET_TYPECHAR:
    //                             buildingToBuild = boost::shared_ptr<Building>(new Turret(this->ownerId, *point));
    //                             break;
    //                     }

    //                     if (buildingToBuild)
    //                     {
    //                         if (game->registerNewEntityIfNoCollision(buildingToBuild))
    //                         {
    //                             setMoveTarget(Target(buildingToBuild), PRIME_TRANSFER_RANGE);
    //                         }
    //                     }
    //                     else
    //                     {
    //                         cout << "Prime refuses to build for that typechar!" << endl;
    //                         state = NotTransferring;
    //                     }
    //                 }
    //                 else if (boost::shared_ptr<Entity> entity = target->castToEntityPtr(*game))
    //                 {
    //                     if (auto building = boost::dynamic_pointer_cast<Building, Entity>(entity))
    //                     {
    //                         if (building->getBuiltRatio() < fixed32(1))
    //                         {
    //                             coinsInt builtAmount = building->build(GOLD_TRANSFER_RATE, &this->heldGold);
    //                             if (builtAmount > 0)
    //                             {
    //                                 goldTransferState_view = BuildingSomething;
    //                             }
    //                         }
    //                         else
    //                         {
    //                             state = NotTransferring;
    //                         }
    //                     }
    //                     else
    //                     {
    //                         cout << "Prime trying to build a non-Building entity... What's going on???" << endl;
    //                     }
    //                 }
    //                 else
    //                 {
    //                     cout << "Can't cast that Target to a position OR an entity..." << endl;
    //                     state = NotTransferring;
    //                 }
    //             }
    //             else
    //             {
    //                 // target far away, do nothing yet
    //             }
    //         }
    //         else
    //         {
    //             // target has been deleted; go back to default mode.
    //             state = NotTransferring;
    //         }
    //     }
    //     break;
    // }
    iterateMobileUnitBasics();
}

// find nearest gateway and deposit to any goldpile it has (creating one if needed)
// void Prime::setStateToReturnGoldOrResetBehavior()
// {
//     Game *game = this->getGameOrThrow();

//     boost::shared_ptr<Gateway> bestChoice;
//     uint32_t bestChoiceDistanceFloorSquared;
//     for (unsigned int i=0; i<game->entities.size(); i++)
//     {
//         if (auto gateway = boost::dynamic_pointer_cast<Gateway, Entity>(game->entities[i]))
//         {
//             if (gateway->isActive() && getAllianceType(this->ownerId, gateway) == Owned)
//             {
//                 uint32_t distanceFloorSquared = (this->getPos() - gateway->getPos()).getFloorMagnitudeSquared();
//                 if (!bestChoice || distanceFloorSquared < bestChoiceDistanceFloorSquared)
//                 {
//                     bestChoice = gateway;
//                     bestChoiceDistanceFloorSquared = distanceFloorSquared;
//                 }
//             }
//         }
//     }

//     if (bestChoice)
//     {
//         // search Gateway queue for existing goldpile
//         for (unsigned int i=0; i<bestChoice->scuttleTargetQueue.size(); i++)
//         {
//             if (auto goldpile = boost::dynamic_pointer_cast<GoldPile, Entity>(game->entities[bestChoice->scuttleTargetQueue[i]]))
//             {
//                 state = PutdownGold;
//                 setMoveTarget(goldpile->getRefOrThrow(), PRIME_TRANSFER_RANGE);
//                 return;
//             }
//         }

//         // if we didn't find one, set target to Gateway; Prime will search again when it gets close.
//         state = PutdownGold;
//         setMoveTarget(bestChoice->getRefOrThrow(), GATEWAY_RANGE);
//         depositingToGateway = true;
//     }
//     else
//     {
//         behavior = Basic;
//         state = NotTransferring;
//         clearMoveTarget();
//         maybeGatherTargetPos = {};
//     }
// }

vector<Coins*> Prime::getDroppableCoins()
{
    return vector<Coins*>{&goldInvested, &heldGold};
}

fixed32 Prime::getHeldGoldRatio()
{
    return ((fixed32)this->heldGold.getInt()) / PRIME_MAX_GOLD_HELD;
}



// ------------------------------------------------------------------------------
// ------------------------------------------------------------------------------
// ------------------------------------------------------------------------------
// ------------------------------------------------------------------------------
// ------------------------------------------------------------------------------
// ------------------------------------------------------------------------------
// ------------------------------------------------------------------------------
// ------------------------------------------------------------------------------
// ------------------------------------------------------------------------------
// ------- COMBATUNIT ------
// ------------------------------------------------------------------------------
// ------------------------------------------------------------------------------
// ------------------------------------------------------------------------------
// ------------------------------------------------------------------------------
// ------------------------------------------------------------------------------
// ------------------------------------------------------------------------------
// ------------------------------------------------------------------------------
// ------------------------------------------------------------------------------
// ------------------------------------------------------------------------------


void packTarget(Netpack::Builder* to, Target target)
{
    target.pack(to);
}
Target consumeTarget(Netpack::Consumer* from)
{
    return Target(from);
}

CombatUnit::CombatUnit()
    : state(NotAttacking), maybeAttackObjective({}), shootCooldown(0), animateShot_view(None), lastShot_view(None)
{}
void CombatUnit::packCombatUnitBasics(Netpack::Builder* to)
{
    to->packEnum(state);
    to->packOptional(maybeAttackObjective, packTarget);
    to->packOptional(maybeAttackTarget, packEntityRef);
    to->packUint16_t(shootCooldown);
}
CombatUnit::CombatUnit(Netpack::Consumer* from)
{
    state = from->consumeEnum<State>();
    maybeAttackObjective = from->consumeOptional(consumeTarget);
    maybeAttackTarget = from->consumeOptional(consumeEntityRef);
    shootCooldown = from->consumeUint16_t();
}

void CombatUnit::cmdAttack(Target target)
{
    if (auto ref = target.castToEntityRef())
    {
        state = AttackingSpecific;
        maybeAttackObjective = target;
        maybeAttackTarget = ref;
    }
    else if (auto point = target.castToPoint())
    {
        if (dynamic_cast<MobileUnit*>(this))
        {
            state = AttackingGeneral;
            maybeAttackObjective = point;
        }
    }
}

bool CombatUnit::tryShootAt(boost::shared_ptr<Unit> targetUnit)
{
    vector2fp toTarget = (targetUnit->getPos() - this->getPos());
    angle_view = static_cast<float>(toTarget.getAngle());
    if (toTarget.getFloorMagnitudeSquared() <= getShotRangeFloorsquared())
    {
        if (shootCooldown == 0)
        {
            shootAt(targetUnit);
            return true;
        }
    }
    return false;
}
fixed32 CombatUnit::calcAttackPriority(boost::shared_ptr<Unit> foreignUnit)
{
    if (auto combatUnit = boost::dynamic_pointer_cast<CombatUnit, Unit>(foreignUnit))
    {
        fixed32 baseCombatUnitPriority(3);
        if (auto otherCombatUnitTargetRef = combatUnit->maybeAttackTarget)
        {
            if (*otherCombatUnitTargetRef == this->getRefOrThrow())
            {
                return baseCombatUnitPriority + 1;
            }
        }
        return baseCombatUnitPriority;
    }
    else if (auto prime = boost::dynamic_pointer_cast<Prime, Unit>(foreignUnit))
    {
        return fixed32(2);
    }
    else
    {
        return fixed32(1);
    }
}
void CombatUnit::shootAt(boost::shared_ptr<Unit> unit)
{
    animateShot_view = (lastShot_view != Left) ? Left : Right;
    lastShot_view = animateShot_view;
    lastShotTargetPos_view = unit->getPos();

    shootCooldown = getShotCooldown();
    unit->takeHit(getShotDamage());
}

bool CombatUnit::combatUnitIsIdle()
{
    return (state == NotAttacking);
}

uint32_t CombatUnit::getShotRangeFloorsquared() const
{
    throw runtime_error("getShotRangeFloorsquared() has not been defined for " + getTypename() + ".\n");
}
uint16_t CombatUnit::getShotCooldown() const
{
    throw runtime_error("getShotCooldown() has not been defined for " + getTypename() + ".\n");
}
uint16_t CombatUnit::getShotDamage() const
{
    throw runtime_error("getShotDamage() has not been defined for " + getTypename() + ".\n");
}
fixed32 CombatUnit::getShotRange() const
{
    throw runtime_error("getShotRange() has not been defined for " + getTypename() + ".\n");
}
fixed32 CombatUnit::getAggressionRange() const
{
    throw runtime_error("getAggressionRange() has not been defined for " + getTypename() + ".\n");
}

void CombatUnit::iterateCombatUnitBasics()
{
    Game *game = getGameOrThrow();

    animateShot_view = None;
    if (shootCooldown > 0)
        shootCooldown --;

    switch (state)
    {
        case NotAttacking:
        {
            if (isIdle())
            {
                auto entitiesInSight = game->entitiesWithinCircle(getPos(), getAggressionRange());

                boost::shared_ptr<Entity> closestValidTarget;
                uint32_t closestDistanceFloorSquared;
                for (unsigned int i=0; i<entitiesInSight.size(); i++)
                {
                    auto entity = entitiesInSight[i];
                    if (getAllianceType(this->ownerId, entity) == Foreign)
                    {
                        uint32_t distanceFloorSquared = (this->getPos() - entity->getPos()).getFloorMagnitudeSquared();
                        if (!closestValidTarget || distanceFloorSquared < closestDistanceFloorSquared)
                        {
                            closestValidTarget = entity;
                            closestDistanceFloorSquared = distanceFloorSquared;
                        }
                    }
                }

                if (closestValidTarget)
                {
                    state = AttackingGeneral;
                    maybeAttackObjective = {};
                    maybeAttackTarget = closestValidTarget->getRefOrThrow();
                }
            }
        }
        break;
        case AttackingGeneral:
        {
            // try to find an attack target, ultimately updating this->maybeAttackTarget
            boost::shared_ptr<Unit> bestTarget;
            fixed32 bestTargetPriority;
            bool alreadyHadTarget = false;

            if (auto attackTarget = maybeAttackTarget)
            {
                if (auto targetUnit = boost::dynamic_pointer_cast<Unit,Entity>(maybeEntityRefToPtrOrNull(*game, *attackTarget)))
                {
                    bestTarget = targetUnit;
                    bestTargetPriority = this->calcAttackPriority(targetUnit);
                    alreadyHadTarget = true; // might be overridden by a higher priority unit nearby, but is sticky in the face of ties
                }
            }

            auto entitiesInSight = game->entitiesWithinCircle(getPos(), getAggressionRange());
            for (unsigned int i=0; i<entitiesInSight.size(); i++)
            {
                if (auto unit = boost::dynamic_pointer_cast<Unit, Entity>(entitiesInSight[i]))
                {
                    if (getAllianceType(this->ownerId, entitiesInSight[i]) == Foreign)
                    {
                        bool thisIsBetterTarget = false; // until proven otherwise

                        fixed32 priority = this->calcAttackPriority(unit);

                        // if there is not yet a bestTarget, or if the priority is higher
                        if ((!bestTarget) || priority > bestTargetPriority)
                        {
                            thisIsBetterTarget = true;
                        }

                        // if the priority matches and we didn't already have a target before this frame...
                        else if (priority == bestTargetPriority && !alreadyHadTarget)
                        {
                            // compare distances
                            uint32_t currentDistanceFloorSquared = (this->getPos() - bestTarget->getPos()).getFloorMagnitudeSquared();
                            uint32_t distanceFloorSquared = (this->getPos() - unit->getPos()).getFloorMagnitudeSquared();
                            if (distanceFloorSquared < currentDistanceFloorSquared)
                                thisIsBetterTarget = true;
                        }

                        if (thisIsBetterTarget)
                        {
                            bestTarget = unit;
                            bestTargetPriority = priority;
                        }
                    }
                }
            }

            if (bestTarget)
                maybeAttackTarget = bestTarget->getRefOrThrow();

            // if we don't have either an objective or target, return to NotAttacking
            if (!(maybeAttackObjective || maybeAttackTarget))
            {
                state = NotAttacking;
                break;
            }

            bool didShoot = false;
            if (maybeAttackTarget)
            {
                if (auto unit = boost::dynamic_pointer_cast<Unit, Entity>(maybeEntityRefToPtrOrNull(*game, *maybeAttackTarget)))
                {
                    didShoot = tryShootAt(unit);
                    if (unit->dead)
                    {
                        maybeAttackTarget = {};
                    }
                }
                else
                {
                    cout << "CombatUnit has a maybeAttackTarget that's not a unit, or is dead or something?? :/ Clearing." << endl;
                    maybeAttackTarget = {};
                }
            }

            if (auto mobileUnitSelf = dynamic_cast<MobileUnit*>(this))
            {
                if (!didShoot)
                {
                    // determine approach target
                    Target approachTarget(EntityRef(0));
                    if (maybeAttackTarget)
                    {
                        approachTarget = Target(*maybeAttackTarget);
                    }
                    else if (maybeAttackObjective)
                    {
                        approachTarget = *maybeAttackObjective;
                    }
                    else {
                        // target probably died
                        state = NotAttacking;
                        break;
                    }
                    
                    // approach the target
                    if (auto point = approachTarget.castToPoint())
                    {
                        mobileUnitSelf->setMoveTarget(*point, fixed32(0));
                        if ((*point - this->getPos()).getFloorMagnitudeSquared() == 0)
                        {
                            state = NotAttacking;
                            maybeAttackObjective = {};
                            maybeAttackTarget = {};
                            break;
                        }
                    }
                    else
                    {
                        mobileUnitSelf->setMoveTarget(approachTarget, getShotRange());
                    }
                }
            }
        }
        break;
        case AttackingSpecific:
            if (auto target = maybeAttackObjective)
            {
                bool returnToIdle = false;
                if (auto targetEntity = target->castToEntityPtr(*game)) // will return false if unit died (pointer will be empty)
                {
                    if (auto targetUnit = boost::dynamic_pointer_cast<Unit, Entity>(targetEntity))
                    {
                        if (auto mobileUnitSelf = dynamic_cast<MobileUnit*>(this))
                        {
                            mobileUnitSelf->setMoveTarget(*target, getRange());
                        }
                        tryShootAt(targetUnit);
                    }
                    else
                        returnToIdle = true;
                }
                else
                    returnToIdle = true;

                if (returnToIdle)
                {
                    if (auto mobileUnitSelf = dynamic_cast<MobileUnit*>(this))
                    {
                        mobileUnitSelf->clearMoveTarget();
                    }
                    state = NotAttacking;
                }
            }
            break;
    }
}

void CombatUnit::combatUnitStop()
{
    state = NotAttacking;
    maybeAttackObjective = {};
    maybeAttackTarget = {};
}


// ------------------------------------------------------------------------------
// ------------------------------------------------------------------------------
// ------------------------------------------------------------------------------
// ------------------------------------------------------------------------------
// ------------------------------------------------------------------------------
// ------------------------------------------------------------------------------
// ------------------------------------------------------------------------------
// ------------------------------------------------------------------------------
// ------------------------------------------------------------------------------
// ------- FIGHTER ------
// ------------------------------------------------------------------------------
// ------------------------------------------------------------------------------
// ------------------------------------------------------------------------------
// ------------------------------------------------------------------------------
// ------------------------------------------------------------------------------
// ------------------------------------------------------------------------------
// ------------------------------------------------------------------------------
// ------------------------------------------------------------------------------
// ------------------------------------------------------------------------------

Fighter::Fighter(uint8_t ownerId, vector2fp pos)
    : Unit(ownerId, FIGHTER_COST, FIGHTER_HEALTH, pos)
{}
void Fighter::pack(Netpack::Builder* to)
{
    packEntityAndUnitBasics(to);
    packMobileUnitBasics(to);
    packCombatUnitBasics(to);
}
Fighter::Fighter(Netpack::Consumer* from)
    : Unit(from)
    , MobileUnit(from)
    , CombatUnit(from)
{}

void Fighter::iterate()
{
    iterateCombatUnitBasics();
    iterateMobileUnitBasics();
}
void Fighter::onMoveCmd(vector2fp moveTo)
{
    state = NotAttacking;
}

uint32_t Fighter::getShotRangeFloorsquared() const { return FIGHTER_SHOT_RANGE_FLOORSQUARED; }
uint16_t Fighter::getShotCooldown() const { return FIGHTER_SHOT_COOLDOWN; }
uint16_t Fighter::getShotDamage() const { return FIGHTER_SHOT_DAMAGE; }
fixed32 Fighter::getShotRange() const { return FIGHTER_SHOT_RANGE; }
fixed32 Fighter::getAggressionRange() const { return FIGHTER_AGGRESSION_RANGE; }
fixed32 Fighter::getRadius() const { return FIGHTER_RADIUS; }
fixed32 Fighter::getMaxSpeed() const { return FIGHTER_SPEED; }
fixed32 Fighter::getRange() const { return FIGHTER_SHOT_RANGE; }
coinsInt Fighter::getCost() const { return FIGHTER_COST; }
uint16_t Fighter::getMaxHealth() const { return FIGHTER_HEALTH; }

uint8_t Fighter::typechar() const { return FIGHTER_TYPECHAR; }
string Fighter::getTypename() const { return "Fighter"; }

bool Fighter::isIdle()
{
    return (mobileUnitIsIdle() && combatUnitIsIdle());
}
void Fighter::cmdStop()
{
    mobileUnitStop();
    combatUnitStop();
}



// ------------------------------------------------------------------------------
// ------------------------------------------------------------------------------
// ------------------------------------------------------------------------------
// ------------------------------------------------------------------------------
// ------------------------------------------------------------------------------
// ------------------------------------------------------------------------------
// ------------------------------------------------------------------------------
// ------------------------------------------------------------------------------
// ------------------------------------------------------------------------------
// ------- TURRET ------
// ------------------------------------------------------------------------------
// ------------------------------------------------------------------------------
// ------------------------------------------------------------------------------
// ------------------------------------------------------------------------------
// ------------------------------------------------------------------------------
// ------------------------------------------------------------------------------
// ------------------------------------------------------------------------------
// ------------------------------------------------------------------------------
// ------------------------------------------------------------------------------

Turret::Turret(uint8_t ownerId, vector2fp pos)
    : Unit(ownerId, TURRET_COST, TURRET_HEALTH, pos)
{}
void Turret::pack(Netpack::Builder* to)
{
    packEntityAndUnitBasics(to);
    packBuildingBasics(to);
    packCombatUnitBasics(to);
}
Turret::Turret(Netpack::Consumer* from)
    : Unit(from)
    , Building(from)
    , CombatUnit(from)
{}

void Turret::iterate()
{
    iterateCombatUnitBasics();
    iterateBuildingBasics();
}

uint32_t Turret::getShotRangeFloorsquared() const { return TURRET_SHOT_RANGE_FLOORSQUARED; }
uint16_t Turret::getShotCooldown() const { return TURRET_SHOT_COOLDOWN; }
uint16_t Turret::getShotDamage() const { return TURRET_SHOT_DAMAGE; }
fixed32 Turret::getShotRange() const { return TURRET_SHOT_RANGE; }
fixed32 Turret::getRadius() const { return TURRET_RADIUS; }
fixed32 Turret::getRange() const { return TURRET_SHOT_RANGE; }
coinsInt Turret::getCost() const { return TURRET_COST; }
uint16_t Turret::getMaxHealth() const { return TURRET_HEALTH; }
fixed32 Turret::getAggressionRange() const { return TURRET_SHOT_RANGE; }

uint8_t Turret::typechar() const { return TURRET_TYPECHAR; }
string Turret::getTypename() const { return "Fighter"; }

bool Turret::isIdle()
{
    return (combatUnitIsIdle());
}
void Turret::cmdStop()
{
    combatUnitStop();
}



// ------------------------------------------------------------------------------
// ------------------------------------------------------------------------------
// ------------------------------------------------------------------------------
// ------------------------------------------------------------------------------
// ------------------------------------------------------------------------------
// ------------------------------------------------------------------------------
// ------------------------------------------------------------------------------
// ------------------------------------------------------------------------------
// ------------------------------------------------------------------------------
// ----- UNPACKENTITY ---
// ------------------------------------------------------------------------------
// ------------------------------------------------------------------------------
// ------------------------------------------------------------------------------
// ------------------------------------------------------------------------------
// ------------------------------------------------------------------------------
// ------------------------------------------------------------------------------
// ------------------------------------------------------------------------------
// ------------------------------------------------------------------------------
// ------------------------------------------------------------------------------



boost::shared_ptr<Entity> consumeEntity(Netpack::Consumer* from)
{
    uint8_t typechar = consumeTypechar(from);
    
    switch (typechar)
    {
    case NULL_TYPECHAR:
        return boost::shared_ptr<Entity>();
        break;
    case GOLDPILE_TYPECHAR:
        return boost::shared_ptr<Entity>(new GoldPile(from));
        break;
    case BEACON_TYPECHAR:
        return boost::shared_ptr<Entity>(new Beacon(from));
        break;
    case GATEWAY_TYPECHAR:
        return boost::shared_ptr<Entity>(new Gateway(from));
        break;
    case PRIME_TYPECHAR:
        return boost::shared_ptr<Entity>(new Prime(from));
        break;
    case FIGHTER_TYPECHAR:
        return boost::shared_ptr<Entity>(new Fighter(from));
        break;
    case TURRET_TYPECHAR:
        return boost::shared_ptr<Entity>(new Turret(from));
        break;
    }
    throw runtime_error("Trying to unpack an unrecognized entity");
}
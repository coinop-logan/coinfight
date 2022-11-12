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
    if (!entity1 && !entity2)
    {
        // both null, good!
        return true;
    }
    else if (bool(entity1) != bool(entity2))
    {
        triggerDebug();
        return false;
    }

    // we now know they're both non-null

    debugAssert(entity1->getPos() == entity2->getPos());
    debugAssert(entity1->dead == entity2->dead);
    // don't need to test regInfo directly
    debugAssert(entity1->getSearchGridCellOrThrow() == entity2->getSearchGridCellOrThrow());
    debugAssert(entity1->typechar() == entity2->typechar());

    bool successfulCast = false; // until proven otherwise

    if (auto goldpile1 = boost::dynamic_pointer_cast<GoldPile, Entity>(entity1))
        if (auto goldpile2 = boost::dynamic_pointer_cast<GoldPile, Entity>(entity2))
    {
        successfulCast = true;
        
        debugAssert(goldpile1->gold.getInt() == goldpile2->gold.getInt());

        debugAssert(successfulCast);
    }

    if (auto unit1 = boost::dynamic_pointer_cast<Unit, Entity>(entity1))
        if (auto unit2 = boost::dynamic_pointer_cast<Unit, Entity>(entity2))
    {
        debugAssert(unit1->getHealthAssumingBuilt() == unit2->getHealthAssumingBuilt());
        debugAssert(unit1->ownerId == unit2->ownerId);
        debugAssert(unit1->goldInvested.getInt() == unit2->goldInvested.getInt());

        if (auto building1 = boost::dynamic_pointer_cast<Building, Unit>(unit1))
            if (auto building2 = boost::dynamic_pointer_cast<Building, Unit>(unit2))
        {} // no building-specific state to check

        if (auto mobileUnit1 = boost::dynamic_pointer_cast<MobileUnit, Unit>(unit1))
            if (auto mobileUnit2 = boost::dynamic_pointer_cast<MobileUnit, Unit>(unit2))
        {
            debugAssert(maybeMoveTargetInfosAreEqual(mobileUnit1->getMaybeMoveTargetInfo(), mobileUnit2->getMaybeMoveTargetInfo()));
            debugAssert(mobileUnit1->getDesiredVelocity() == mobileUnit2->getDesiredVelocity());
            debugAssert(mobileUnit1->getLastVelocity() == mobileUnit2->getLastVelocity());
        }

        if (auto combatUnit1 = boost::dynamic_pointer_cast<CombatUnit, Unit>(unit1))
            if (auto combatUnit2 = boost::dynamic_pointer_cast<CombatUnit, Unit>(unit2))
        {
            debugAssert(combatUnit1->state == combatUnit2->state);
            debugAssert(combatUnit1->maybeAttackObjective == combatUnit2->maybeAttackObjective);
            debugAssert(combatUnit1->maybeAttackTarget == combatUnit2->maybeAttackTarget);
            debugAssert(combatUnit1->shootCooldown == combatUnit2->shootCooldown);
        }

        if (auto beacon1 = boost::dynamic_pointer_cast<Beacon, Unit>(unit1))
            if (auto beacon2 = boost::dynamic_pointer_cast<Beacon, Unit>(unit2))
        {
            successfulCast = true;

            debugAssert(beacon1->state == beacon2->state);
        }

        if (auto gateway1 = boost::dynamic_pointer_cast<Gateway, Unit>(unit1))
            if (auto gateway2 = boost::dynamic_pointer_cast<Gateway, Unit>(unit2))
        {
            successfulCast = true;

            debugAssert(gateway1->maybeDepositingPrime == gateway2->maybeDepositingPrime);
            debugAssert(gateway1->maybeWithdrawingPrime == gateway2->maybeWithdrawingPrime);
            debugAssert(gateway1->buildTargetQueue == gateway2->buildTargetQueue);
            debugAssert(gateway1->scuttleTargetQueue == gateway2->scuttleTargetQueue);
        }

        if (auto prime1 = boost::dynamic_pointer_cast<Prime, Unit>(unit1))
            if (auto prime2 = boost::dynamic_pointer_cast<Prime, Unit>(unit2))
        {
            successfulCast = true;

            debugAssert(prime1->heldGold.getInt() == prime2->heldGold.getInt());
            debugAssert(prime1->fundsSource == prime2->fundsSource);
            debugAssert(prime1->fundsDest == prime2->fundsDest);
            debugAssert(prime1->scavengeTargetQueue == prime2->scavengeTargetQueue);
            debugAssert(prime1->buildTargetQueue == prime2->buildTargetQueue);
        }

        if (auto fighter1 = boost::dynamic_pointer_cast<Fighter, Unit>(unit1))
            if (auto fighter2 = boost::dynamic_pointer_cast<Fighter, Unit>(unit2))
        {
            successfulCast = true;
        }

        if (auto turret1 = boost::dynamic_pointer_cast<Turret, Unit>(unit1))
            if (auto turret2 = boost::dynamic_pointer_cast<Turret, Unit>(unit2))
        {
            successfulCast = true;
        }

        debugAssert(successfulCast);
    }

    return debugAssert(successfulCast);
    // this repetition of debugAssert may seem redundant,
    // but this way, breakpoints trigger in a more informative position
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

bool Gateway::isInBuildTargetQueue(EntityRef entityRef)
{
    for (unsigned int i=0; i<buildTargetQueue.size(); i++)
    {
        if (entityRef == buildTargetQueue[i])
        {
            return true;
        }
    }
    return false;
}
bool Gateway::isInScuttleTargetQueue(EntityRef entityRef)
{
    for (unsigned int i=0; i<scuttleTargetQueue.size(); i++)
    {
        if (entityRef == scuttleTargetQueue[i])
        {
            return true;
        }
    }
    return false;
}

optional<tuple<EntityRef, bool>> Gateway::getMaybeAbsorbTarget()
{
    if (maybeDepositingPrime)
    {
        return {{*maybeDepositingPrime, false}};
    }
    else if (scuttleTargetQueue.size() > 0)
    {
        return {{scuttleTargetQueue[0], true}};
    }
    else
    {
        return {};
    }
}
optional<tuple<EntityRef, bool>> Gateway::getMaybeDepositTarget()
{
    if (maybeWithdrawingPrime)
    {
        return {{*maybeWithdrawingPrime, false}};
    }
    else if (buildTargetQueue.size() > 0)
    {
        return {{buildTargetQueue[0], true}};
    }
    else
    {
        return {};
    }
}

bool Gateway::requestDepositFromPrime(Prime* prime)
{
    if (maybeDepositingPrime)
    {
        if (prime->getRefOrThrow() == *maybeDepositingPrime)
        {
            return true;
        }
        else return false;
    }
    else
    {
        if (prime->heldGold.getInt() > 0)
        {
            maybeDepositingPrime = prime->getRefOrThrow();
            return true;
        }
        else
        {
            return false;
        }
    }
}
bool Gateway::requestWithdrawFromPrime(Prime* prime)
{
    if (maybeWithdrawingPrime)
    {
        if (prime->getRefOrThrow() == *maybeWithdrawingPrime)
        {
            return true;
        }
        else return false;
    }
    else
    {
        if (prime->heldGold.getSpaceLeft() > 0)
        {
            maybeWithdrawingPrime = prime->getRefOrThrow();
            return true;
        }
        else
        {
            return false;
        }
    }
}

void Gateway::cancelAnyDepositRequestFromPrime(Prime* prime)
{
    if (maybeDepositingPrime && *maybeDepositingPrime == prime->getRefOrThrow())
    {
        maybeDepositingPrime = {};
    }
}
void Gateway::cancelAnyWithdrawRequestFromPrime(Prime* prime)
{
    if (maybeWithdrawingPrime && *maybeWithdrawingPrime == prime->getRefOrThrow())
    {
        maybeWithdrawingPrime = {};
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
void Gateway::cmdDepositTo(EntityRef entityRef)
{
    if (buildTargetQueue.size() >= 255)
        return;
    
    buildTargetQueue.push_back(entityRef);
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
      goldFlowFrom_view({{}, false}), goldFlowTo_view({{}, false})
{}
void Gateway::pack(Netpack::Builder* to)
{
    packEntityAndUnitBasics(to);
    packBuildingBasics(to);

    to->packOptional<EntityRef>(maybeDepositingPrime, packEntityRef);
    to->packOptional<EntityRef>(maybeWithdrawingPrime, packEntityRef);

    if (buildTargetQueue.size() > 255)
        cout << "WARNING! Gateway's buildTargetQueue.size() is greater than 255; pack will fail!" << endl;
    if (scuttleTargetQueue.size() > 255)
        cout << "WARNING! Gateway's scuttleTargetQueue.size() is greater than 255; pack will fail!" << endl;
    
    to->packUint8_t(buildTargetQueue.size());
    for (unsigned int i=0; i<buildTargetQueue.size(); i++)
    {
        packEntityRef(to, buildTargetQueue[i]);
    }
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
    maybeDepositingPrime = from->consumeOptional<EntityRef>(consumeEntityRef);
    maybeWithdrawingPrime = from->consumeOptional<EntityRef>(consumeEntityRef);

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

void Gateway::validateTargets()
{
    Game* game = getGameOrThrow();

    if (maybeDepositingPrime)
    {
        if (auto prime = boost::dynamic_pointer_cast<Prime, Entity>(game->entities[*maybeDepositingPrime]))
        {
            if ((prime->getPos() - this->getPos()).getFloorMagnitudeSquared() > PRIME_TRANSFER_RANGE_FLOORSQUARED)
            {
                maybeDepositingPrime = {};
            }
            if (prime->heldGold.getInt() == 0)
            {
                maybeDepositingPrime = {};
            }
        }
        else
        {
            maybeDepositingPrime = {};
        }
    }
    if (maybeWithdrawingPrime)
    {
        if (auto prime = boost::dynamic_pointer_cast<Prime, Entity>(game->entities[*maybeWithdrawingPrime]))
        {
            if ((prime->getPos() - this->getPos()).getFloorMagnitudeSquared() > PRIME_TRANSFER_RANGE_FLOORSQUARED)
            {
                maybeWithdrawingPrime = {};
            }
            if (prime->heldGold.getSpaceLeft() == 0)
            {
                maybeWithdrawingPrime = {};
            }
        }
        else
        {
            maybeWithdrawingPrime = {};
        }
    }

    // Let's iterate through and remove out of range units IF they're not both mobile and moving to the GW
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

    // let's make sure no goldpiles are "blocking" any other finite jobs in the queue
    if (buildTargetQueue.size() > 0)
    {
        if (auto firstGoldpile = boost::dynamic_pointer_cast<GoldPile, Entity>(game->entities[buildTargetQueue[0]]))
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
    }

    // okay, we now know that
    //   - all entities in the queue are either in range or moving toward the GW
    //   - all entities in the queue are non-null
    //   - if any entities are in-range, the first one is at the head of the queue
    //   - if a goldpile is at the head of the list, and there are non-gold-piles, we've moved the head gold pile to the end
}

void Gateway::iterate()
{
    validateTargets();

    // until proven otherwise
    goldFlowFrom_view = {{}, false};
    goldFlowTo_view = {{}, false};

    Game *game = getGameOrThrow();

    auto maybeAbsorbTargetInfo = getMaybeAbsorbTarget();
    auto maybeDepositTargetInfo = getMaybeDepositTarget();

    // maybe there's something to scuttle
    if (maybeAbsorbTargetInfo)
    {
        EntityRef absorbTarget = get<0>(*maybeAbsorbTargetInfo);
        bool tryingToScuttle = get<1>(*maybeAbsorbTargetInfo);

        auto entity = game->entities[absorbTarget];

        // check it's in range
        if ((this->getPos() - entity->getPos()).getFloorMagnitudeSquared() <= GATEWAY_RANGE_FLOORSQUARED)
        {
            // we have something to scuttle!
            coinsInt amountPulled(0);
            bool scuttling = false;

            if (auto goldPile = boost::dynamic_pointer_cast<GoldPile, Entity>(entity))
            {
                amountPulled = goldPile->gold.transferUpTo(GOLD_TRANSFER_RATE, &game->players[this->ownerId].credit);
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
                if (!pulledFromPrimeHeldGold && tryingToScuttle)
                {
                    amountPulled = unit->unbuild(GOLD_TRANSFER_RATE, &game->players[this->ownerId].credit);
                    scuttling = true;
                }
            }

            if (amountPulled > 0)
            {
                goldFlowFrom_view = {entity, scuttling};
            }
            else
            {
                // can't scuttle no more, remove from queue
                scuttleTargetQueue.erase(scuttleTargetQueue.begin());
            }   
        }
    }

    // if we have money, maybe there's something to build?
    if (game->players[this->ownerId].credit.getInt() > 0 && maybeDepositTargetInfo)
    {
        EntityRef depositTarget = get<0>(*maybeDepositTargetInfo);
        bool fromQueue = get<1>(*maybeDepositTargetInfo);

        if (auto entity = game->entities[depositTarget])
        {
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
                        goldFlowTo_view = {entity, (bool(maybeBuildingUnit))};
                    }
                }
                else
                {
                    if (fromQueue)
                    {
                        buildTargetQueue.erase(buildTargetQueue.begin());
                    }
                    else
                    {
                        maybeDepositingPrime = {};
                    }
                }
            }
            else // entity out of range, remove from list
            {
                if (fromQueue)
                {
                    buildTargetQueue.erase(buildTargetQueue.begin());
                }
                else
                {
                    maybeDepositingPrime = {};
                }
            }
        }
        else // entity was dead or null or something; remove from list
        {
            if (fromQueue)
            {
                buildTargetQueue.erase(buildTargetQueue.begin());
            }
            else
            {
                maybeDepositingPrime = {};
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
      goldFlowFrom_view({{}, false}), goldFlowTo_view({{}, false})
{}
void Prime::pack(Netpack::Builder* to)
{
    packEntityAndUnitBasics(to);
    packMobileUnitBasics(to);

    heldGold.pack(to);

    to->packOptional<EntityRef>(fundsSource, packEntityRef);
    to->packOptional<EntityRef>(fundsDest, packEntityRef);

    if (buildTargetQueue.size() > 255)
        cout << "WARNING! Prime's buildTargetQueue.size() is greater than 255; pack will fail!" << endl;
    if (scavengeTargetQueue.size() > 255)
        cout << "WARNING! Prime's scuttleTargetQueue.size() is greater than 255; pack will fail!" << endl;
    
    to->packUint8_t(scavengeTargetQueue.size());
    for (unsigned int i=0; i<scavengeTargetQueue.size(); i++)
    {
        scavengeTargetQueue[i].pack(to);
    }
    to->packUint8_t(buildTargetQueue.size());
    for (unsigned int i=0; i<buildTargetQueue.size(); i++)
    {
        packEntityRef(to, buildTargetQueue[i]);
    }
}
Prime::Prime(Netpack::Consumer* from)
    : Unit(from)
    , MobileUnit(from),
    heldGold(PRIME_MAX_GOLD_HELD)
{
    heldGold = Coins(from);

    fundsSource = from->consumeOptional<EntityRef>(consumeEntityRef);
    fundsDest = from->consumeOptional<EntityRef>(consumeEntityRef);

    auto scavengeTargetQueueSize = from->consumeUint8_t();
    scavengeTargetQueue.reserve(scavengeTargetQueueSize);
    for (unsigned int i=0; i<scavengeTargetQueueSize; i++)
    {
        Target target(from);
        scavengeTargetQueue.push_back(target);
    }

    auto buildTargetQueueSize = from->consumeUint8_t();
    buildTargetQueue.reserve(buildTargetQueueSize);
    for (unsigned int i=0; i<buildTargetQueueSize; i++)
    {
        EntityRef ref = consumeEntityRef(from);
        buildTargetQueue.push_back(ref);
    }
}

void Prime::cancelAnyFetchesFrom(Target target)
{
    if (auto gateway = boost::dynamic_pointer_cast<Gateway, Entity>(target.castToEntityPtr(*getGameOrThrow())))
    {
        gateway->cancelAnyWithdrawRequestFromPrime(this);
    }
    if (fundsSource && fundsSource == target.castToEntityRef())
    {
        fundsSource = {};
    }
    for (unsigned int i=0; i<scavengeTargetQueue.size(); i++)
    {
        if (scavengeTargetQueue[i] == target)
        {
            scavengeTargetQueue.erase(scavengeTargetQueue.begin() + i);
            i --;
            continue;
        }
    }
}
void Prime::cancelAnyDepositsTo(Target target)
{
    if (auto gateway = boost::dynamic_pointer_cast<Gateway, Entity>(target.castToEntityPtr(*getGameOrThrow())))
    {
        gateway->cancelAnyDepositRequestFromPrime(this);
    }
    if (auto entityRef = target.castToEntityRef())
    {
        if (fundsDest && *fundsDest == entityRef)
        {
            fundsDest = {};
        }
        for (unsigned int i=0; i<buildTargetQueue.size(); i++)
        {
            if (buildTargetQueue[i] == entityRef)
            {
                buildTargetQueue.erase(buildTargetQueue.begin() + i);
                i --;
                continue;
            }
        }
    }
}

void Prime::cmdDeposit(EntityRef entityRef)
{
    cancelAnyFetchesFrom(Target(entityRef));

    if (auto entity = getGameOrThrow()->entities[entityRef])
    {
        if (auto goldpile = boost::dynamic_pointer_cast<GoldPile, Entity>(entity))
        {
            this->fundsDest = goldpile->getRefOrThrow();
            setMoveTarget(Target(entityRef), PRIME_TRANSFER_RANGE);
        }
        else if (auto unit = boost::dynamic_pointer_cast<Unit, Entity>(entity))
        {
            if (unit->getBuiltRatio() < fixed32(1))
            {
                if (buildTargetQueue.size() < 255)
                {
                    buildTargetQueue.insert(buildTargetQueue.begin(), unit->getRefOrThrow());
                    setMoveTarget(Target(entityRef), PRIME_TRANSFER_RANGE);
                }
            }
            else
            {
                if (unit->typechar() == GATEWAY_TYPECHAR || unit->typechar() == PRIME_TYPECHAR)
                {
                    this->fundsDest = unit->getRefOrThrow();
                    setMoveTarget(Target(entityRef), PRIME_TRANSFER_RANGE);
                }
                else
                {
                    // This is a built unit that is not a Gateway or Prime. Ignore.
                }
            }
        }
    }
    else
    {
        // unit has died.
    }
}

void Prime::cmdFetch(Target _target)
{
    cancelAnyDepositsTo(_target);

    if (auto point = _target.castToPoint())
    {
        if (scavengeTargetQueue.size() < 255)
        {
            scavengeTargetQueue.insert(scavengeTargetQueue.begin(), _target);
            setMoveTarget(_target, PRIME_TRANSFER_RANGE);
        }
    }
    else if (auto entity = _target.castToEntityPtr(*getGameOrThrow()))
    {
        if (entity->typechar() == GOLDPILE_TYPECHAR)
        {
            if (scavengeTargetQueue.size() < 255)
            {
                scavengeTargetQueue.insert(scavengeTargetQueue.begin(), _target);
                setMoveTarget(_target, PRIME_TRANSFER_RANGE);
            }
        }
        else if (entity->typechar() == GATEWAY_TYPECHAR || entity->typechar() == PRIME_TYPECHAR)
        {
            this->fundsSource = entity->getRefOrThrow();
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

optional<tuple<Target, bool>> Prime::getMaybeFetchTarget() // boolean indicates whether we want to scuttle
{
    if (scavengeTargetQueue.size() > 0)
    {
        return {{scavengeTargetQueue[0], true}};
    }
    else if (fundsSource)
    {
        return {{Target(*fundsSource), false}};
    }
    else
    {
        return {};
    }
}
optional<tuple<EntityRef, bool>> Prime::getMaybeDepositTarget() // boolean indicates whether we want to build
{
    if (buildTargetQueue.size() > 0)
    {
        return {{buildTargetQueue[0], true}};
    }
    else if (fundsDest)
    {
        return {{*fundsDest, false}};
    }
    else
    {
        return {};
    }
}

bool Prime::isInScavengeTargetQueue(Target target)
{
    for (unsigned int i=0; i<scavengeTargetQueue.size(); i++)
    {
        if (target == scavengeTargetQueue[i])
        {
            return true;
        }
    }
    return false;
}
bool Prime::isInBuildTargetQueue(EntityRef entityRef)
{
    for (unsigned int i=0; i<buildTargetQueue.size(); i++)
    {
        if (entityRef == buildTargetQueue[i])
        {
            return true;
        }
    }
    return false;
}

void Prime::validateTargets()
{
    Game* game = getGameOrThrow();
    // Remove invalid targets, as well as build orders that are completed

    if (fundsSource && ! game->entities[*fundsSource])
    {
        // entity died. Clear it
        fundsSource = {};
    }
    
    if (fundsDest && ! game->entities[*fundsDest])
    {
        // entity died. Clear it
        fundsDest = {};
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
        if (auto goldpile = boost::dynamic_pointer_cast<GoldPile, Entity>(scavengeTargetQueue[i].castToEntityPtr(*game)))
        {
            if (goldpile->gold.getInt() == 0)
            {
                scavengeTargetQueue.erase(scavengeTargetQueue.begin() + i);
                i --;
                continue;
            }
        }
    }
}

void Prime::tryTransferAndMaybeMoveOn()
{
    Game* game = getGameOrThrow();

    // Transfer (possibly to and from, both at once), and handle removing targets or switching moveTarget
    auto maybeFetchTargetInfo = getMaybeFetchTarget();
    auto maybeDepositTargetInfo = getMaybeDepositTarget();

    // First, pull from the fetchTarget if possible
    if (maybeFetchTargetInfo)
    {
        Target fetchTarget = get<0>(*maybeFetchTargetInfo);
        bool tryingToScuttle = get<1>(*maybeFetchTargetInfo);

        if (heldGold.getSpaceLeft() > 0)
        {
            if (auto fetchTargetPoint = fetchTarget.getPointUnlessTargetDeleted(*game))
            {
                if ((this->getPos() - *fetchTargetPoint).getFloorMagnitudeSquared() <= PRIME_TRANSFER_RANGE_FLOORSQUARED)
                {
                    if (auto entity = fetchTarget.castToEntityPtr(*game))
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
                                bool pullingFromPrimeOrGateway = false;
                                if (auto prime = boost::dynamic_pointer_cast<Prime, Unit>(unit))
                                {
                                    if (prime->heldGold.getInt() > 0)
                                    {
                                        amountPulled = prime->heldGold.transferUpTo(GOLD_TRANSFER_RATE, &this->heldGold);
                                        pullingFromPrimeOrGateway = true;
                                    }
                                }
                                else if (auto gateway = boost::dynamic_pointer_cast<Gateway, Unit>(unit))
                                {
                                    if (!tryingToScuttle)
                                    {
                                        if (gateway->requestWithdrawFromPrime(this))
                                        {
                                            pullingFromPrimeOrGateway = true;
                                        }
                                    }
                                }

                                if (!pullingFromPrimeOrGateway && tryingToScuttle)
                                {
                                    amountPulled = unit->unbuild(GOLD_TRANSFER_RATE, &this->heldGold);
                                    scuttling = true;
                                }
                            }

                            if (amountPulled > 0)
                            {
                                goldFlowFrom_view = {entity, scuttling};
                            }
                        }
                    }
                    else
                    {
                        // we've arrived at the fetch target point. Remove it from the list.
                        if (scavengeTargetQueue.size() > 0 && fetchTarget == scavengeTargetQueue[0])
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
        }

        if (getMaybeMoveTarget() == fetchTarget)
        {
            if (heldGold.getSpaceLeft() == 0)
            {
                if (maybeDepositTargetInfo)
                {
                    if (auto entity = game->entities[get<0>(*maybeDepositTargetInfo)])
                    {
                        auto depositTargetPoint = entity->getPos();
                        if ((this->getPos() - depositTargetPoint).getFloorMagnitudeSquared() > PRIME_TRANSFER_RANGE_FLOORSQUARED)
                        {
                            setMoveTarget(Target(entity->getRefOrThrow()), PRIME_TRANSFER_RANGE);
                        }
                    }
                }
            }
        }
    }

    // Now deposit to something if we can
    if (maybeDepositTargetInfo)
    {
        Target depositTarget = get<0>(*maybeDepositTargetInfo);
        // bool tryingToBuild = get<1>(*maybeDepositTargetInfo);

        if (heldGold.getInt() > 0)
        {
            if (auto depositTargetPoint = depositTarget.getPointUnlessTargetDeleted(*game))
            {
                if ((this->getPos() - *depositTargetPoint).getFloorMagnitudeSquared() <= PRIME_TRANSFER_RANGE_FLOORSQUARED)
                {
                    if (auto entity = depositTarget.castToEntityPtr(*game))
                    {
                        coinsInt amountPushed(0);
                        bool building = false;
                        boost::shared_ptr<Entity> entityPushedTo;

                        if (auto goldpile = boost::dynamic_pointer_cast<GoldPile, Entity>(entity))
                        {
                            amountPushed = this->heldGold.transferUpTo(GOLD_TRANSFER_RATE, &goldpile->gold);
                            entityPushedTo = goldpile;
                        }
                        else if (auto unit = boost::dynamic_pointer_cast<Unit, Entity>(entity))
                        {
                            if (unit->getBuiltRatio() < fixed32(1))
                            {
                                amountPushed = unit->build(GOLD_TRANSFER_RATE, &this->heldGold);
                                entityPushedTo = unit;
                                building = true;
                            }
                            else
                            {
                                if (auto gateway = boost::dynamic_pointer_cast<Gateway, Unit>(unit))
                                {
                                    bool success = gateway->requestDepositFromPrime(this);
                                    if (!success)
                                    {
                                        // If we can't directly deposit, create a new gold pile and add it to the Gateway's scuttle queue.
                                        // but first, check for an existing one close enough by, so we don't clutter the area with tiny gold piles.
                                        boost::shared_ptr<GoldPile> gpToDepositTo;
                                        for (unsigned int i=0; i<gateway->scuttleTargetQueue.size(); i++)
                                        {
                                            if (auto goldpile = boost::dynamic_pointer_cast<GoldPile, Entity>(game->entities[gateway->scuttleTargetQueue[i]]))
                                            {
                                                if ((goldpile->getPos() - this->getPos()).getFloorMagnitudeSquared() < PRIME_TRANSFER_RANGE_FLOORSQUARED * 1.1)
                                                {
                                                    gpToDepositTo = goldpile;
                                                }
                                            }
                                        }

                                        if (!gpToDepositTo)
                                        {
                                            vector2fp gpPos = (gateway->getPos() + this->getPos()) / 2;
                                            gpToDepositTo = boost::shared_ptr<GoldPile>(new GoldPile(gpPos));
                                            game->registerNewEntityIgnoringCollision(gpToDepositTo);
                                            gateway->scuttleTargetQueue.push_back(gpToDepositTo->getRefOrThrow());
                                        }

                                        amountPushed = this->heldGold.transferUpTo(GOLD_TRANSFER_RATE, &gpToDepositTo->gold);
                                        entityPushedTo = gpToDepositTo;
                                    }
                                }
                                else if (auto prime = boost::dynamic_pointer_cast<Prime, Unit>(unit))
                                {
                                    amountPushed = this->heldGold.transferUpTo(GOLD_TRANSFER_RATE, &prime->heldGold);
                                    entityPushedTo = prime;
                                }
                                else
                                {
                                    cout << "Warning: somehow a Prime is trying to deposit to a built unit other than a GW or Prime." << endl;
                                }
                            }
                        }

                        if (amountPushed > 0)
                        {
                            goldFlowTo_view = {entityPushedTo, building};
                        }
                    }
                    else
                    {
                        cout << "Warning: logic error related to Prime build queue." << endl;
                    }
                }
            }   
        }

        if (getMaybeMoveTarget() == depositTarget)
        {
            if (heldGold.getInt() == 0)
            {
                if (maybeFetchTargetInfo)
                {
                    auto fetchTarget = get<0>(*maybeFetchTargetInfo);
                    if (auto fetchTargetPoint = fetchTarget.getPointUnlessTargetDeleted(*game))
                    {
                        fixed32 targetRange = fetchTarget.type == Target::PointTarget ?
                            fixed32(0) :
                            PRIME_TRANSFER_RANGE ;
                        if ((this->getPos() - *fetchTargetPoint).getRoughMagnitude() > targetRange)
                        {
                                
                            setMoveTarget(get<0>(*maybeFetchTargetInfo), targetRange);
                        }
                    }
                    else
                    {
                        cout << "Logic error. Can't cast fetchTarget to point, but this should have been validated earlier in Prime::iterate..." << endl;
                    }
                }
            }
        }
    }
}

void Prime::iterate()
{
    validateTargets();

    // until proven otherwise
    goldFlowFrom_view = {{}, false};
    goldFlowTo_view = {{}, false};

    Game *game = getGameOrThrow();

    optional<Target> fetchTarget;
    if (auto maybeFetchTarget = getMaybeFetchTarget())
    {
        fetchTarget = get<0>(*maybeFetchTarget);
    }
    optional<Target> depositTarget;
    if (auto maybeDepositTarget = getMaybeDepositTarget())
    {
        depositTarget = get<0>(*maybeDepositTarget);
    }

    // We have three main sources of state, with which to implement higher-level logic (through reading and modifying):
    //   fetchTarget (optional)
    //   depositTarget (optional)
    //   MobileUnit::moveTarget (optional)

    // moveTarget represents our mose immediate action: moving toward some target or picking up / depositing something once we're there.

    if (!mobileUnitIsIdle())
    {
        // if we're still moving somewhere, there's nothing for us to do
        // ... unless we're doing a "fetch to"
        if (fetchTarget && fetchTarget->type == Target::PointTarget && getMaybeMoveTarget() && fetchTarget->castToPoint() == getMaybeMoveTarget()->castToPoint())
        {
            auto entitiesInSight = game->entitiesWithinCircle(this->getPos(), PRIME_SIGHT_RANGE);
            auto goldpilesInSight = filterForType<GoldPile, Entity>(entitiesInSight);

            for (unsigned int i=0; i<goldpilesInSight.size(); i++)
            {
                auto goldpile = goldpilesInSight[i];
                
                if (!(isInBuildTargetQueue(goldpile->getRefOrThrow()) || isInScavengeTargetQueue(goldpile->getRefOrThrow())))
                {
                    Target gpTarget = Target(goldpile->getRefOrThrow());
                    scavengeTargetQueue.insert(scavengeTargetQueue.begin(), gpTarget);
                    if (scavengeTargetQueue.size() > 255)
                    {
                        scavengeTargetQueue.pop_back();
                    }
                    setMoveTarget(gpTarget, PRIME_TRANSFER_RANGE);
                }
            }
        }
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

    iterateMobileUnitBasics();
}

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
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
    if (!entity1 && entity2)
    {
        triggerDebug();
        return false;
    }
    if (entity1 && !entity2)
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
    }

    if (auto unit1 = boost::dynamic_pointer_cast<Unit, Entity>(entity1))
        if (auto unit2 = boost::dynamic_pointer_cast<Unit, Entity>(entity2))
    {
        debugAssert(unit1->getHealth() == unit2->getHealth());
        debugAssert(unit1->ownerId == unit2->ownerId);
        debugAssert(unit1->goldInvested.getInt() == unit2->goldInvested.getInt());

        if (auto building1 = boost::dynamic_pointer_cast<Building, Unit>(unit1))
            if (auto building2 = boost::dynamic_pointer_cast<Building, Unit>(unit2))
        {
            // no building-specific state to check

            if (auto beacon1 = boost::dynamic_pointer_cast<Beacon, Building>(building1))
                if (auto beacon2 = boost::dynamic_pointer_cast<Beacon, Building>(building2))
            {
                successfulCast = true;

                debugAssert(beacon1->state == beacon2->state);
            }

            if (auto gateway1 = boost::dynamic_pointer_cast<Gateway, Building>(building1))
                if (auto gateway2 = boost::dynamic_pointer_cast<Gateway, Building>(building2))
            {
                successfulCast = true;

                debugAssert(gateway1->state == gateway2->state);
                debugAssert(gateway1->maybeTargetEntity == gateway2->maybeTargetEntity);
            }

            debugAssert(successfulCast);
        }

        if (auto mobileUnit1 = boost::dynamic_pointer_cast<MobileUnit, Unit>(unit1))
            if (auto mobileUnit2 = boost::dynamic_pointer_cast<MobileUnit, Unit>(unit2))
        {
            debugAssert(maybeMoveTargetInfosAreEqual(mobileUnit1->getMaybeMoveTargetInfo(), mobileUnit2->getMaybeMoveTargetInfo()));
            debugAssert(mobileUnit1->getDesiredVelocity() == mobileUnit2->getDesiredVelocity());
            debugAssert(mobileUnit1->getLastVelocity() == mobileUnit2->getLastVelocity());

            if (auto prime1 = boost::dynamic_pointer_cast<Prime, MobileUnit>(mobileUnit1))
                if (auto prime2 = boost::dynamic_pointer_cast<Prime, MobileUnit>(mobileUnit2))
            {
                successfulCast = true;

                debugAssert(prime1->heldGold.getInt() == prime2->heldGold.getInt());
                debugAssert(prime1->behavior == prime2->behavior);
                debugAssert(prime1->maybeGatherTargetPos == prime2->maybeGatherTargetPos);
                debugAssert(prime1->state == prime2->state);
                debugAssert(prime1->gonnabuildTypechar == prime2->gonnabuildTypechar);
            }

            if (auto fighter1 = boost::dynamic_pointer_cast<Fighter, MobileUnit>(mobileUnit1))
                if (auto fighter2 = boost::dynamic_pointer_cast<Fighter, MobileUnit>(mobileUnit2))
            {
                successfulCast = true;

                debugAssert(fighter1->state == fighter2->state);
                debugAssert(maybeTargetsAreEqual(fighter1->state, fighter2->state));
            }

            debugAssert(successfulCast);
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

Unit::Unit() : Entity() {} // this will throw if called. Needed for virtual inheritance later but should never be called.

Unit::Unit(uint8_t ownerId, coinsInt totalCost, uint16_t health, vector2fp pos)
    : Entity(pos), health(health), ownerId(ownerId), goldInvested(totalCost) {}
void Unit::packEntityAndUnitBasics(Netpack::Builder* to)
{
    packEntityBasics(to);

    to->packUint8_t(ownerId);
    to->packUint16_t(health);
    goldInvested.pack(to);
}
Unit::Unit(Netpack::Consumer* from)
    : Entity(from),
      goldInvested((coinsInt)0), // will get overwritten with consume below
      angle_view(0)
{
    ownerId = from->consumeUint8_t();
    health = from->consumeUint16_t();
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
    if (damage >= health)
    {
        health = 0;
        dead = true;
    }
    else
    {
        health -= damage;
    }
}
uint16_t Unit::getHealth() { return health; }



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
        desiredVelocity = unitDir * distanceLeft;
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
uint16_t Beacon::getMaxHealth() const { return BEACON_HEALTH; }

Beacon::Beacon(uint8_t ownerId, vector2fp pos, State state)
    : Unit(ownerId, GATEWAY_COST, BEACON_HEALTH, pos)
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
    return state == Idle;
}

void Gateway::cmdBuildUnit(uint8_t unitTypechar)
{
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
            state = DepositTo;
            getGameOrThrow()->registerNewEntityIgnoringCollision(littleBabyUnitAwwwwSoCute);
            this->maybeTargetEntity = littleBabyUnitAwwwwSoCute->getRefOrThrow();
        }
    }
}
void Gateway::cmdDepositTo(Target target)
{
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
            maybeTargetEntity = goldpile->getRefOrThrow();
            state = DepositTo;
        }
        else if (auto entityRef = target.castToEntityRef())
        {
            maybeTargetEntity = *entityRef;
            state = DepositTo;
        }
        else
        {
            cout << "Gateway can't cast that Target to a point or entity during cmdDepositTo" << endl;
        }
    }
}
void Gateway::cmdScuttle(EntityRef targetRef)
{
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
                if (auto unit = boost::dynamic_pointer_cast<Unit, Entity>(entity))
                {
                    if (getAllianceType(this->ownerId, unit) == Owned)
                    {
                        if ((this->getPos() - unit->getPos()).getFloorMagnitudeSquared() > GATEWAY_RANGE_FLOORSQUARED)
                        {
                            if (auto mobileUnit = boost::dynamic_pointer_cast<MobileUnit, Unit>(unit))
                            {
                                mobileUnit->setMoveTarget(this->getRefOrThrow(), GATEWAY_RANGE);
                                maybeTargetEntity = targetRef;
                                state = Scuttle;
                            }
                            else
                            {
                                // Building out of range; ignore
                            }
                        }
                        else
                        {
                            maybeTargetEntity = targetRef;
                            state = Scuttle;
                        }
                    }
                    else
                    {
                        // Not owned by player; just ignore
                    }
                }
                else if (auto goldpile = boost::dynamic_pointer_cast<GoldPile, Entity>(entity))
                {
                    if ((this->getPos() - goldpile->getPos()).getFloorMagnitudeSquared() > GATEWAY_RANGE_FLOORSQUARED)
                    {
                        // too far away!
                        state = Idle;
                        maybeTargetEntity = {};
                    }
                    else
                    {
                        state = Scuttle;
                        maybeTargetEntity = {goldpile->getRefOrThrow()};
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

fixed32 Gateway::buildQueueWeight()
{
    if (!isActive())
        return fixed32(10);
    else if (state == Idle)
        return fixed32(0);
    else
        return fixed32(1);
}

Gateway::Gateway(uint8_t ownerId, vector2fp pos)
    : Unit(ownerId, GATEWAY_COST, GATEWAY_HEALTH, pos),
      state(Idle), inGameTransferState_view(NoGoldTransfer)
{}
void Gateway::pack(Netpack::Builder* to)
{
    packEntityAndUnitBasics(to);
    packBuildingBasics(to);

    to->packEnum(state);
    to->packOptional(maybeTargetEntity, packEntityRef);
}
Gateway::Gateway(Netpack::Consumer* from)
    : Unit(from)
    , Building(from)
{
    state = from->consumeEnum<Gateway::State>();
    maybeTargetEntity = from->consumeOptional(consumeEntityRef);
}

void Gateway::iterate()
{
    Game *game = getGameOrThrow();

    inGameTransferState_view = NoGoldTransfer; // will possibly be updated in the following switch
    switch (state)
    {
        case Idle:
        {
            // search for units near enough to complete
            vector<EntityRef> nearbyEntityRefs = game->searchGrid.nearbyEntitiesSloppyIncludingEmpty(this->getPos(), GATEWAY_RANGE);

            for (unsigned int i=0; i<nearbyEntityRefs.size(); i++)
            {
                boost::shared_ptr<Entity> entity = maybeEntityRefToPtrOrNull(*game, {nearbyEntityRefs[i]});

                if (auto unit = boost::dynamic_pointer_cast<Unit, Entity>(entity))
                {
                    if (unit->ownerId == this->ownerId)
                        if (unit->getBuiltRatio() < fixed32(1))
                            if ((unit->getPos() - this->getPos()).getFloorMagnitudeSquared() <= GATEWAY_RANGE_FLOORSQUARED)
                                {
                                    state = DepositTo;
                                    maybeTargetEntity = {unit->getRefOrThrow()};
                                }
                }
            }
        }
        break;
        case DepositTo:
        {
            if (boost::shared_ptr<Entity> depositingToEntityPtr = maybeEntityRefToPtrOrNull(*game, maybeTargetEntity))
            {
                // stop if it's out of range
                if ((depositingToEntityPtr->getPos() - this->getPos()).getFloorMagnitudeSquared() > GATEWAY_RANGE_FLOORSQUARED)
                {
                    state = Idle;
                    maybeTargetEntity = {};
                }
                else
                {
                    Coins* maybeCoinsToDepositTo = NULL;
                    boost::shared_ptr<Unit> maybeBuildingUnit;
                    if (auto goldpile = boost::dynamic_pointer_cast<GoldPile, Entity>(depositingToEntityPtr))
                    {
                        maybeCoinsToDepositTo = &goldpile->gold;
                    }
                    else if (auto unit = boost::dynamic_pointer_cast<Unit, Entity>(depositingToEntityPtr))
                    {
                        if (unit->getBuiltRatio() < fixed32(1))
                        {
                            maybeCoinsToDepositTo = &unit->goldInvested;
                            maybeBuildingUnit = unit;
                        }
                        else if (auto gateway = boost::dynamic_pointer_cast<Gateway, Unit>(unit))
                        {
                            maybeCoinsToDepositTo = &game->players[gateway->ownerId].credit;
                        }
                        else if (auto prime = boost::dynamic_pointer_cast<Prime, Unit>(unit))
                        {
                            maybeCoinsToDepositTo = &prime->heldGold;
                        }
                    }

                    if (maybeCoinsToDepositTo)
                    {
                        coinsInt amountDeposited = game->players[this->ownerId].credit.transferUpTo(GATEWAY_BUILD_RATE, maybeCoinsToDepositTo);
                        if (maybeBuildingUnit && maybeBuildingUnit->getBuiltRatio() == fixed32(1))
                        {
                            state = Idle;
                            maybeTargetEntity = {};
                        }
                        if (amountDeposited == 0)
                        {
                            state = Idle;
                            maybeTargetEntity = {};
                        }
                        if (amountDeposited > 0)
                        {
                            inGameTransferState_view = Pushing;
                        }
                    }
                }
            }
        }
        break;
        case Scuttle:
        {
            if (auto entity = (maybeEntityRefToPtrOrNull(*game, maybeTargetEntity)))
            {
                if ((this->getPos() - entity->getPos()).getFloorMagnitudeSquared() > GATEWAY_RANGE_FLOORSQUARED)
                {
                    if (auto mobileUnit = boost::dynamic_pointer_cast<MobileUnit, Entity>(entity))
                    {
                        if ((!(mobileUnit->getMaybeMoveTarget())) || *(mobileUnit->getMaybeMoveTarget()->castToEntityRef()) != this->getRefOrThrow())
                        {
                            // unit is no longer on its way; revert to Idle state
                            state = Idle;
                            maybeTargetEntity = {};
                        }
                    }
                    else
                    {
                        cout << "somehow a non-mobile unit ended up out of range for a scuttle cmd..." << endl;
                        state = Idle;
                        maybeTargetEntity = {};
                    }
                }
                else
                {
                    coinsInt amountScuttled(0);
                    if (auto goldPile = boost::dynamic_pointer_cast<GoldPile, Entity>(entity))
                    {
                        amountScuttled = goldPile->gold.transferUpTo(GATEWAY_SCUTTLE_RATE, &game->players[this->ownerId].credit);
                    }
                    else if (auto unit = boost::dynamic_pointer_cast<Unit, Entity>(entity))
                    {
                        amountScuttled = unit->unbuild(GATEWAY_SCUTTLE_RATE, &game->players[this->ownerId].credit);
                    }

                    if (amountScuttled > 0)
                    {
                        inGameTransferState_view = Pulling;
                    }
                    else
                    {
                        state = Idle;
                        maybeTargetEntity = {};
                    }
                }
            }
        }
        break;
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




Prime::Prime(uint8_t ownerId, vector2fp pos)
    : Unit(ownerId, PRIME_COST, PRIME_HEALTH, pos),
      heldGold(PRIME_MAX_GOLD_HELD),
      behavior(Basic), maybeGatherTargetPos({}), state(NotTransferring), goldTransferState_view(NoGoldTransfer), gonnabuildTypechar(NULL_TYPECHAR)
{}
void Prime::pack(Netpack::Builder* to)
{
    packEntityAndUnitBasics(to);
    packMobileUnitBasics(to);

    heldGold.pack(to);
    to->packEnum(behavior);
    to->packOptional(maybeGatherTargetPos, packVector2fp);
    to->packEnum(state);
    packTypechar(to, gonnabuildTypechar);
}
Prime::Prime(Netpack::Consumer* from)
    : Unit(from)
    , MobileUnit(from),
    heldGold(PRIME_MAX_GOLD_HELD)
{
    heldGold = Coins(from);
    behavior = from->consumeEnum<Behavior>();
    maybeGatherTargetPos = from->consumeOptional(consumeVector2fp);
    state = from->consumeEnum<State>();
    gonnabuildTypechar = consumeTypechar(from);
}

void Prime::cmdPickup(Target _target)
{
    behavior = Basic;
    state = PickupGold;

    setMoveTarget(_target, PRIME_TRANSFER_RANGE);
}
void Prime::cmdPutdown(Target _target)
{
    behavior = Basic;
    state = PutdownGold;

    setMoveTarget(_target, PRIME_TRANSFER_RANGE);
}
void Prime::cmdBuild(uint8_t buildTypechar, vector2fp buildPos)
{
    behavior = Basic;
    state = Build;
    gonnabuildTypechar = buildTypechar;

    setMoveTarget(buildPos, PRIME_TRANSFER_RANGE);
}
void Prime::cmdResumeBuilding(EntityRef targetUnit)
{
    behavior = Basic;
    state = Build;

    setMoveTarget(Target(targetUnit), PRIME_TRANSFER_RANGE);
}
void Prime::cmdGather(vector2fp targetPos)
{
    behavior = Gather;
    state = NotTransferring;
    maybeGatherTargetPos = {targetPos};
    setMoveTarget(targetPos, fixed32(0));
}
void Prime::cmdScuttle(EntityRef targetUnit)
{
    #warning prime doesnt know how to scuttle yet
}

fixed32 Prime::getHeldGoldRatio()
{
    return ((fixed32)this->heldGold.getInt()) / PRIME_MAX_GOLD_HELD;
}

fixed32 Prime::getRadius() const { return PRIME_RADIUS; }
fixed32 Prime::getMaxSpeed() const { return PRIME_SPEED; }
fixed32 Prime::getRange() const { return PRIME_TRANSFER_RANGE; }
coinsInt Prime::getCost() const { return PRIME_COST; }
uint16_t Prime::getMaxHealth() const { return PRIME_HEALTH; }

uint8_t Prime::typechar() const { return PRIME_TYPECHAR; }
string Prime::getTypename() const { return "Prime"; }

bool Prime::isIdle()
{
    return (mobileUnitIsIdle() && (behavior == Basic && state == NotTransferring));
}

void Prime::iterate()
{
    Game *game = getGameOrThrow();

    // first we process behavior. This is upstream from state.
    switch (behavior)
    {
        case Basic:
            break;
        case Gather:
        {
            if (auto gatherTargetPos = maybeGatherTargetPos)
            {
                // the state indicates what part of the cycle the Prime is in:
                    // NotTransferring: moving toward the gatherTargetPos
                    // PickupGold: moving toward or picking up some gold it found
                    // PutodownGold: bringing Gold to gateway or depositing
                
                // our job here is to switch these states when necessary...

                // firstly, if heldGold is maxed, return gold to nearest gateway
                if (getHeldGoldRatio() == fixed32(1))
                {
                    setStateToReturnGoldOrResetBehavior();
                }

                switch (state)
                {
                    case NotTransferring:
                    {
                        // in theory this should already be set, but in some cases doesn't seem to be
                        setMoveTarget(*gatherTargetPos, fixed32(0));
                        
                        // scan for any GoldPile and choose the closest
                        boost::shared_ptr<GoldPile> bestTarget;
                        uint16_t bestTargetDistanceFloorSquared;

                        auto entitiesInSight = game->entitiesWithinCircle(getPos(), PRIME_SIGHT_RANGE);
                        for (unsigned int i=0; i<entitiesInSight.size(); i++)
                        {
                            if (auto goldPile = boost::dynamic_pointer_cast<GoldPile, Entity>(entitiesInSight[i]))
                            {
                                uint16_t distanceFloorSquared = (this->getPos() - goldPile->getPos()).getFloorMagnitudeSquared();
                                if (!bestTarget || distanceFloorSquared < bestTargetDistanceFloorSquared)
                                {
                                    bestTarget = goldPile;
                                    bestTargetDistanceFloorSquared = distanceFloorSquared;
                                }
                            }
                        }

                        // if we found something, set moveTarget to it and switch state to PickupGold
                        if (bestTarget)
                        {
                            state = PickupGold;
                            setMoveTarget(bestTarget->getRefOrThrow(), PRIME_TRANSFER_RANGE);
                            break;
                        }
                        // otoh, if we've arrived at the target without finding Gold to pickup...
                        else if ((*gatherTargetPos - this->getPos()).getFloorMagnitudeSquared() == 0)
                        {
                            // if we have no Gold, return to basic/idle behavior
                            if (heldGold.getInt() == 0)
                            {
                                behavior = Basic;
                                state = NotTransferring;
                                maybeGatherTargetPos = {};
                                break;
                            }
                            // otherwise, make a return trip
                            else
                            {
                                setStateToReturnGoldOrResetBehavior();
                            }
                        }
                        else
                        {
                            // just to be explicit, in this case we haven't found any gold
                            // but we are still traveling to the destination
                            // do nothing.
                        }
                    }
                    break;
                    case PickupGold:
                    {
                        // note that if heldGoldRatio == 1, we will have caught this just before this switch block,
                        // so we can assume that heldGoldRatio < 1 for now, and focus on gathering more.

                        bool shouldMoveOn = true; // until proven otherwise

                        // here, we need to do something either when the Prime is full or when its current Target gets depleted
                        if (auto moveTarget = getMaybeMoveTarget())
                        {
                            if (auto goldPile = boost::dynamic_pointer_cast<GoldPile, Entity>(moveTarget->castToEntityPtr(*game)))
                            {
                                if (goldPile->gold.getInt() > 0)
                                {
                                    shouldMoveOn = false;
                                }
                            }
                        }

                        if (shouldMoveOn)
                        {
                            state = NotTransferring;
                            setMoveTarget(*gatherTargetPos, fixed32(0));
                            break;
                        }
                        
                    }
                    break;
                    case PutdownGold:
                    {
                        if (heldGold.getInt() == 0)
                        {
                            state = NotTransferring;
                            setMoveTarget(*gatherTargetPos, fixed32(0));
                            break;
                        }
                        // as long as Gateawy (in moveTarget) is still valid, just continue til heldGold == 0
                        bool stillMovingTowardGateway = false; // until proven otherwise
                        if (auto moveTarget = getMaybeMoveTarget())
                        {
                            if (auto gateway = boost::dynamic_pointer_cast<Gateway, Entity>(moveTarget->castToEntityPtr(*game)))
                            {
                                stillMovingTowardGateway = true;
                            }
                        }

                        if (!stillMovingTowardGateway)
                        {
                            // find another gateway
                            setStateToReturnGoldOrResetBehavior();
                        }
                    }
                    break;
                    default:
                    {
                        cout << "Prime is in an unexpected combo of behavior and state. Returning to basic behavior." << endl;
                        behavior = Basic;
                    }
                    break;
                }
            }
            else
            {
                cout << "Behavior is Gather, but there is no gatherPos..." << endl;
                behavior = Basic;
                state = NotTransferring;
                break;
            }
        }
            break;
    }

    goldTransferState_view = NoGoldTransfer;
    switch (state)
    {
    case NotTransferring:
        break;
    case PickupGold:
        if (auto target = getMaybeMoveTarget())
        {
            if (boost::shared_ptr<Entity> e = target->castToEntityPtr(*game))
            {
                if ((e->getPos() - this->getPos()).getFloorMagnitudeSquared() <= PRIME_TRANSFER_RANGE_FLOORSQUARED)
                {
                    optional<Coins*> coinsToPullFrom;
                    if (auto goldpile = boost::dynamic_pointer_cast<GoldPile, Entity>(e))
                    {
                        coinsToPullFrom = &goldpile->gold;
                    }
                    else if (auto gateway = boost::dynamic_pointer_cast<Gateway, Entity>(e))
                    {
                        if (gateway->ownerId == this->ownerId)
                            coinsToPullFrom = &game->players[gateway->ownerId].credit;
                    }
                    if (coinsToPullFrom)
                    {
                        coinsInt pickedUp = (*coinsToPullFrom)->transferUpTo(PRIME_PICKUP_RATE, &(this->heldGold));
                        if (pickedUp == 0)
                            state = NotTransferring;
                        else
                            goldTransferState_view = Pulling;
                    }
                }
            }
        }
        break;
    case PutdownGold:
        if (auto target = getMaybeMoveTarget())
        if (optional<vector2fp> point = target->getPointUnlessTargetDeleted(*game))
        {
            if ((*point - getPos()).getFloorMagnitudeSquared() <= PRIME_TRANSFER_RANGE_FLOORSQUARED)
            {
                optional<Coins*> coinsToPushTo;
                bool stopOnTransferZero = false;
                if (auto entity = target->castToEntityPtr(*game))
                {
                    if (auto goldpile = boost::dynamic_pointer_cast<GoldPile, Entity>(entity))
                    {
                        coinsToPushTo = &goldpile->gold;
                    }
                    else if (auto unit = boost::dynamic_pointer_cast<Unit, Entity>(entity))
                    {
                        // first try to complete it if it's not yet built
                        if (unit->getBuiltRatio() < fixed32(1))
                        {
                            coinsToPushTo = &unit->goldInvested;
                            stopOnTransferZero = true;
                        }
                        else if (auto gateway = boost::dynamic_pointer_cast<Gateway, Unit>(unit))
                        {
                            if (gateway->ownerId == this->ownerId)
                            {
                                coinsToPushTo = &game->players[gateway->ownerId].credit;
                            }
                        }
                        else if (auto prime = boost::dynamic_pointer_cast<Prime, Unit>(unit))
                        {
                            coinsToPushTo = &prime->heldGold;
                        }
                        else
                        {
                            // nothing else to do then.
                            state = NotTransferring;
                        }
                    }
                    else {
                        cout << "not sure how to execute a PutdownGold cmd for a unit other than a gateway or goldpile" << endl;
                        state = NotTransferring;
                    }
                }
                else
                {
                    // must create goldPile
                    boost::shared_ptr<GoldPile> gp(new GoldPile(*point));
                    game->registerNewEntityIgnoringCollision(gp);
                    coinsToPushTo = &gp->gold;
                    setMoveTarget(Target(gp->getRefOrThrow()), PRIME_TRANSFER_RANGE);
                }

                if (coinsToPushTo)
                {
                    coinsInt amountPutDown = this->heldGold.transferUpTo(PRIME_PUTDOWN_RATE, (*coinsToPushTo));
                    if (amountPutDown == 0 && stopOnTransferZero)
                    {
                        state = NotTransferring;
                    }
                    if (amountPutDown != 0)
                    {
                        goldTransferState_view = Pushing;
                    }
                }
                else
                {
                    cout << "I'm confused about how to execute this putdownGold cmd." << endl;
                    state = NotTransferring;
                }
            }
        }
        break;
    case Build:
        if (auto target = getMaybeMoveTarget())
        {
            if (optional<vector2fp> point = target->castToPoint())
            {
                if ((*point - getPos()).getFloorMagnitudeSquared() <= PRIME_TRANSFER_RANGE_FLOORSQUARED)
                {
                    // create unit if typechar checks out and change target to new unit
                    boost::shared_ptr<Building> buildingToBuild;
                    switch (gonnabuildTypechar)
                    {
                        case GATEWAY_TYPECHAR:
                            buildingToBuild = boost::shared_ptr<Building>(new Gateway(this->ownerId, *point));
                            break;
                    }

                    if (buildingToBuild)
                    {
                        if (game->registerNewEntityIfNoCollision(buildingToBuild))
                        {
                            setMoveTarget(Target(buildingToBuild), PRIME_TRANSFER_RANGE);
                        }
                    }
                    else
                    {
                        cout << "Prime refuses to build for that typechar!" << endl;
                        state = NotTransferring;
                    }
                }
            }
            else if (boost::shared_ptr<Entity> entity = target->castToEntityPtr(*game))
            {
                if (auto building = boost::dynamic_pointer_cast<Building, Entity>(entity))
                {
                    if (building->getBuiltRatio() < fixed32(1))
                    {
                        coinsInt builtAmount = building->build(PRIME_PUTDOWN_RATE, &this->heldGold);
                        if (builtAmount > 0)
                        {
                            goldTransferState_view = Pushing;
                        }
                    }
                    else
                    {
                        state = NotTransferring;
                    }
                }
                else
                {
                    cout << "Prime trying to build a non-Building entity... What's going on???" << endl;
                }
            }
            else
            {
                cout << "Can't cast that Target to a position OR an entity..." << endl;
                state = NotTransferring;
            }
        }
        break;
    }
    iterateMobileUnitBasics();
}

void Prime::setStateToReturnGoldOrResetBehavior()
{
    Game *game = this->getGameOrThrow();

    // find nearest gateway and bring gold to it
    boost::shared_ptr<Gateway> bestChoice;
    uint32_t bestChoiceDistanceFloorSquared;
    for (unsigned int i=0; i<game->entities.size(); i++)
    {
        if (auto gateway = boost::dynamic_pointer_cast<Gateway, Entity>(game->entities[i]))
        {
            if (getAllianceType(this->ownerId, gateway) == Owned)
            {
                uint32_t distanceFloorSquared = (this->getPos() - gateway->getPos()).getFloorMagnitudeSquared();
                if (!bestChoice || distanceFloorSquared < bestChoiceDistanceFloorSquared)
                {
                    bestChoice = gateway;
                    bestChoiceDistanceFloorSquared = distanceFloorSquared;
                }
            }
        }
    }

    if (bestChoice)
    {
        state = PutdownGold;
        setMoveTarget(bestChoice->getRefOrThrow(), PRIME_TRANSFER_RANGE);
    }
    else
    {
        behavior = Basic;
        state = NotTransferring;
        clearMoveTarget();
        maybeGatherTargetPos = {};
    }
}

void Prime::onMoveCmd(vector2fp moveTo)
{
    behavior = Basic;
    state = NotTransferring;
}

vector<Coins*> Prime::getDroppableCoins()
{
    return vector<Coins*>{&goldInvested, &heldGold};
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
    : state(NotAttacking), maybeAttackingTarget({}), shootCooldown(0), animateShot(None), lastShot(None)
{}
void CombatUnit::packCombatUnitBasics(Netpack::Builder* to)
{
    to->packEnum(state);
    to->packOptional(maybeAttackingTarget, packTarget);
    to->packUint16_t(shootCooldown);
}
CombatUnit::CombatUnit(Netpack::Consumer* from)
{
    state = from->consumeEnum<State>();
    maybeAttackingTarget = from->consumeOptional(consumeTarget);
    shootCooldown = from->consumeUint16_t();
}

void CombatUnit::cmdAttack(Target target)
{
    if (auto ref = target.castToEntityRef())
    {
        state = AttackingSpecific;
        if (MobileUnit* mobileUnitSidecast = dynamic_cast<MobileUnit*>(this))
        {
            mobileUnitSidecast->setMoveTarget(target, FIGHTER_SHOT_RANGE);
        }
    }
    else if (auto point = target.castToPoint())
    {
        if (MobileUnit* mobileUnitSidecast = dynamic_cast<MobileUnit*>(this))
        {
            state = AttackingGeneral;
            maybeAttackingTarget = point;
            mobileUnitSidecast->setMoveTarget(target, fixed32(0));
        }
    }
}

void CombatUnit::tryShootAt(boost::shared_ptr<Unit> targetUnit)
{
    vector2fp toTarget = (targetUnit->getPos() - this->getPos());
    angle_view = static_cast<float>(toTarget.getAngle());
    if (toTarget.getFloorMagnitudeSquared() <= FIGHTER_SHOT_RANGE_FLOORSQUARED)
    {
        if (shootCooldown == 0)
        {
            shootAt(targetUnit);
            animateShot = (lastShot != Left) ? Left : Right;
            lastShot = animateShot;
        }
    }
}
fixed32 CombatUnit::calcAttackPriority(boost::shared_ptr<Unit> foreignUnit)
{
    if (auto building = boost::dynamic_pointer_cast<Building, Unit>(foreignUnit))
    {
        return fixed32(1);
    }
    else if (auto prime = boost::dynamic_pointer_cast<Prime, Unit>(foreignUnit))
    {
        return fixed32(2);
    }
    else if (auto fighter = boost::dynamic_pointer_cast<Fighter, Unit>(foreignUnit))
    {
        fixed32 baseFighterPriority(3);
        if (auto otherFighterTarget = fighter->getMaybeMoveTarget())
        {
            if (auto otherFighterTargetRef = otherFighterTarget->castToEntityRef())
            {
                if (*otherFighterTargetRef == this->getRefOrThrow())
                {
                    return baseFighterPriority + 1;
                }
            }
        }
        return baseFighterPriority;
    }
    else
    {
        cout << "I can't find the priority of that unit!" << endl;
        return fixed32(0);
    }
}
void CombatUnit::shootAt(boost::shared_ptr<Unit> unit)
{
    shootCooldown = FIGHTER_SHOT_COOLDOWN;
    unit->takeHit(FIGHTER_DAMAGE);
}

bool CombatUnit::combatUnitIsIdle()
{
    return (state == NotAttacking);
}

fixed32 CombatUnit::getAggressionRange() const
{
    throw runtime_error("getAggressionRange() has not been defined for " + getTypename() + ".\n");
}

void CombatUnit::iterateCombatUnitBasics()
{
    Game *game = getGameOrThrow();

    animateShot = None;
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
                    maybeAttackingTarget = Target(closestValidTarget->getRefOrThrow());
                }
            }
        }
        break;
        case AttackingGeneral:
        {
            if (auto attackingTarget = maybeAttackingTarget)
            {
                // first make sure we're not going after some dead unit
                if (!attackingTarget->isStillValid(*game))
                {
                    state = NotAttacking;
                    attackingTarget = {};
                    break;
                }

                // if we're a mobile unit, make sure we're approaching the target
                if (auto mobileUnitSelf = dynamic_cast<MobileUnit*>(this))
                {
                    if (attackingTarget->type == Target::PointTarget)
                    {
                        mobileUnitSelf->setMoveTarget(*attackingTarget, fixed32(0));
                    }
                    else
                    {
                        mobileUnitSelf->setMoveTarget(*attackingTarget, getRange());
                    }
                }

                // if the target is a point and we've arrived (or if we're a building so we can't move), go back to Idle
                if (auto point = attackingTarget->castToPoint())
                {
                    bool isBuilding = (dynamic_cast<Building*>(this));
                    if (isBuilding || (*point - this->getPos()).getFloorMagnitudeSquared() == 0)
                    {
                        state = NotAttacking;
                        attackingTarget = {};
                        break;
                    }
                }

                // at this point we know:
                    // the target is either a live unit or a position we haven't arrived at
                    // we are moving toward the target or staying within range
                
                // we must also keep in mind that moveTarget may be different than attackingTarget
                    // (the former changes more often, the latter is the Fighter's "higher order" command)
                    // the moveTarget is what the Figher is attacking "right now"
                
                // we will search for good options for attack
                boost::shared_ptr<Unit> bestTarget;
                fixed32 bestTargetPriority;
                bool alreadyHadTarget = false;
                if (auto unit = boost::dynamic_pointer_cast<Unit,Entity>(attackingTarget->castToEntityPtr(*game)))
                {
                    bestTarget = unit;
                    bestTargetPriority = this->calcAttackPriority(unit);
                    alreadyHadTarget = true; // might be overridden by a higher priority unit nearby, but is sticky in the face of ties
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
                {
                    if (auto mobileUnitSelf = dynamic_cast<MobileUnit*>(this))
                    {
                        mobileUnitSelf->setMoveTarget(Target(bestTarget), FIGHTER_SHOT_RANGE);
                    }
                    tryShootAt(bestTarget);
                }
            }
            else
            {
                cout << "state is attackingGeneral, but there is no attackingTarget..." << endl;
                state = NotAttacking;
            }
        }
        break;
        case AttackingSpecific:
            if (auto target = maybeAttackingTarget)
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

fixed32 Fighter::getRadius() const { return FIGHTER_RADIUS; }
fixed32 Fighter::getMaxSpeed() const { return FIGHTER_SPEED; }
fixed32 Fighter::getRange() const { return FIGHTER_SHOT_RANGE; }
coinsInt Fighter::getCost() const { return FIGHTER_COST; }
uint16_t Fighter::getMaxHealth() const { return FIGHTER_HEALTH; }
fixed32 Fighter::getAggressionRange() const { return FIGHTER_SIGHT_RANGE; }

uint8_t Fighter::typechar() const { return FIGHTER_TYPECHAR; }
string Fighter::getTypename() const { return "Fighter"; }

bool Fighter::isIdle()
{
    return (mobileUnitIsIdle() && combatUnitIsIdle());
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
    }
    throw runtime_error("Trying to unpack an unrecognized entity");
}
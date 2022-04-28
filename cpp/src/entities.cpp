#include <boost/shared_ptr.hpp>
#include "engine.h"
#include "entities.h"




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



void Target::pack(vch *dest)
{
    packToVch(dest, "C", (unsigned char)(type));

    packVector2f(dest, pointTarget);
    packEntityRef(dest, entityTarget);
}
void Target::unpackAndMoveIter(vchIter *iter)
{
    unsigned char enumInt;
    *iter = unpackFromIter(*iter, "C", &enumInt);
    type = static_cast<Type>(enumInt);

    *iter = unpackVector2f(*iter, &pointTarget);
    *iter = unpackEntityRef(*iter, &entityTarget);
}

Target::Target(vchIter *iter)
{
    unpackAndMoveIter(iter);
}
Target::Target(vector2f _pointTarget)
{
    type = PointTarget;
    pointTarget = _pointTarget;
}
Target::Target(EntityRef _entityTarget)
{
    type = EntityTarget;
    pointTarget = vector2f(0, 0);
    entityTarget = _entityTarget;
}
Target::Target(boost::shared_ptr<Entity> entity)
    : Target(entity->getRefOrThrow()) {}

bool Target::isStillValid(const Game &game)
{
    return
        (type == PointTarget) ||
        (maybeEntityRefToPtrOrNull(game, {entityTarget}) != NULL);
}
optional<vector2f> Target::getPointUnlessTargetDeleted(const Game &game)
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

optional<vector2f> Target::castToPoint()
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

unsigned char getMaybeNullEntityTypechar(boost::shared_ptr<Entity> e)
{
    if (e)
        return e->typechar();
    else
        return NULL_TYPECHAR;
}

AllianceType getAllianceType(int playerIdOrNegativeOne, boost::shared_ptr<Entity> entity)
{
    if (auto goldPile = boost::dynamic_pointer_cast<GoldPile, Entity>(entity))
    {
        return Neutral;
    }
    else if (auto unit = boost::dynamic_pointer_cast<Unit, Entity>(entity))
    {
        if (playerIdOrNegativeOne == unit->ownerId)
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


vector2f Entity::getPos() const
{
    return pos;
}
float Entity::getRadius() const
{
    throw runtime_error("getRadius() has not been defined for " + getTypeName() + ".\n");
}
void Entity::setPosAndUpdateCell(vector2f newPos)
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
unsigned char Entity::typechar() const
{
    throw runtime_error("typechar() has not been defined for " + getTypeName() + "\n");
}
string Entity::getTypeName() const
{
    throw runtime_error("getTypeName() has not been defined for this unit.\n");
}

bool Entity::collidesWithPoint(vector2f point)
{
    return (pos - point).getMagnitude() <= ENTITY_COLLIDE_RADIUS;
}

void Entity::iterate()
{
    throw runtime_error("go() has not been defined for " + getTypeName() + ".\n");
}
sf::Color Entity::getTeamOrPrimaryColor()
{
    throw runtime_error("getTeamOrPrimaryColor() has not been defined for " + getTypeName() + ".\n");
}
void Entity::pack(vch *dest)
{
    throw runtime_error("pack() has not been defined for " + getTypeName() + ".\n");
}
void Entity::unpackAndMoveIter(vchIter *iter, Game &game)
{
    throw runtime_error("unpackMoveIter() has not been defined for " + getTypeName() + ".\n");
}
void Entity::packEntity(vch *destVch)
{
    packToVch(destVch, "C", (unsigned char)dead);

    packVector2f(destVch, pos);
}
void Entity::unpackEntityAndMoveIter(vchIter *iter)
{
    unsigned char deadChar;
    *iter = unpackFromIter(*iter, "C", &deadChar);
    dead = (bool)deadChar;

    *iter = unpackVector2f(*iter, &pos);
}
Entity::Entity(vector2f pos) : pos(pos),
                               dead(false)
                               
{}
Entity::Entity(vchIter *iter)
{
    unpackEntityAndMoveIter(iter);
}
void Entity::die()
{
    dead = true;
}
vector<Coins*> Entity::getDroppableCoins()
{
    throw runtime_error("getDroppableCoins has not been defined for " + getTypeName() + ".");
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
void GoldPile::pack(vch *dest)
{
    packEntity(dest);
    gold.pack(dest);
}
void GoldPile::unpackAndMoveIter(vchIter *iter)
{
    gold = Coins(iter);
}
sf::Color GoldPile::getTeamOrPrimaryColor()
{
    return sf::Color(sf::Color::Yellow);
}

GoldPile::GoldPile(vector2f pos) : Entity(pos),
                                   gold(MAX_COINS)
{}
GoldPile::GoldPile(vchIter *iter) : Entity(iter),
                                    gold(MAX_COINS)
{
    unpackAndMoveIter(iter);
}

float GoldPile::getRadius() const { return 10; }
unsigned char GoldPile::typechar() const { return GOLDPILE_TYPECHAR; }
string GoldPile::getTypeName() const { return "GoldPile"; }
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
    throw runtime_error("getCost() has not been defined for " + getTypeName() + ".");
}
uint16_t Unit::getMaxHealth() const
{
    throw runtime_error("getMaxHeatlh() has not been defined for " + getTypeName() + ".");
}
void Unit::packUnit(vch *destVch)
{
    packEntity(destVch);

    packToVch(destVch, "C", (unsigned char)ownerId);
    packToVch(destVch, "H", health);

    goldInvested.pack(destVch);
}

void Unit::unpackUnitAndMoveIter(vchIter *iter)
{
    unsigned char ownerIdChar;
    *iter = unpackFromIter(*iter, "C", &ownerIdChar);
    ownerId = ownerIdChar;

    *iter = unpackFromIter(*iter, "H", &health);

    goldInvested = Coins(iter);
}

Unit::Unit(int ownerId, coinsInt totalCost, uint16_t health, vector2f pos)
    : Entity(pos), health(health), ownerId(ownerId), goldInvested(totalCost) {}

Unit::Unit(vchIter *iter)
    : Entity(iter),
      goldInvested((coinsInt)0) // will get overwritten in unpack below
{
    unpackUnitAndMoveIter(iter);
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
float Unit::getBuiltRatio()
{
    return (float)getBuilt() / getCost();
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
void Unit::unitIterate() {}
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



void Building::packBuilding(vch *destVch)
{
    packUnit(destVch);
}
void Building::unpackBuildingAndMoveIter(vchIter *iter)
{
}

Building::Building(int ownerId, coinsInt totalCost, uint16_t health, vector2f pos)
    : Unit(ownerId, totalCost, health, pos) {}
Building::Building(vchIter *iter) : Unit(iter)
{
    unpackBuildingAndMoveIter(iter);
}

void Building::buildingIterate()
{
    unitIterate();
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


void MoveTargetInfo::pack(vch *dest)
{
    target.pack(dest);
    packToVch(dest, "fff", desiredRange, closestDistanceSquared, frustration);
}
void MoveTargetInfo::unpackAndMoveIter(vchIter *iter)
{
    target = Target(iter);
    *iter = unpackFromIter(*iter, "fff", &desiredRange, &closestDistanceSquared, &frustration);
}
MoveTargetInfo::MoveTargetInfo(Target target, float desiredRange, float closestDistanceSquared)
    : target(target), desiredRange(desiredRange), closestDistanceSquared(closestDistanceSquared), frustration(0)
    {}
MoveTargetInfo::MoveTargetInfo(vchIter *iter) : target((EntityRef)0)
{
    unpackAndMoveIter(iter);
}



void MobileUnit::packMobileUnit(vch *dest)
{
    packUnit(dest);
    if (auto targetInfo = maybeTargetInfo)
    {
        packTrue(dest);

        targetInfo->pack(dest);
    }
    else
    {
        packFalse(dest);
    }

    packVector2f(dest, desiredVelocity);
    packVector2f(dest, lastVelocity);
}
void MobileUnit::unpackMobileUnitAndMoveIter(vchIter *iter)
{
    if (unpackBoolAndMoveIter(iter))
    {
        MoveTargetInfo targetInfo(iter);
        maybeTargetInfo = {targetInfo};
    }
    else
    {
        maybeTargetInfo = {};
    }

    *iter = unpackVector2f(*iter, &desiredVelocity);
    *iter = unpackVector2f(*iter, &lastVelocity);
}

MobileUnit::MobileUnit(int ownerId, coinsInt totalCost, uint16_t health, vector2f pos)
    : Unit(ownerId, totalCost, health, pos), maybeTargetInfo({}), desiredVelocity(vector2f(0,0)), lastVelocity(vector2f(0,0)), angle_view(0)
{}
MobileUnit::MobileUnit(vchIter *iter) : Unit(iter),
                                        angle_view(0)
{
    unpackMobileUnitAndMoveIter(iter);
}

float MobileUnit::getMaxSpeed() const
{
    throw runtime_error("getMaxSpeed has not been defined for '" + getTypeName() + "'");
}
float MobileUnit::getRange() const
{
    throw runtime_error("getRange has not been defined for '" + getTypeName() + "'");
}
vector2f MobileUnit::getDesiredVelocity() const
{
    return desiredVelocity;
}
vector2f MobileUnit::getLastVelocity() const
{
    return lastVelocity;
}
void MobileUnit::onMoveCmd(vector2f moveTo)
{
    throw runtime_error("onMoveCmd() has not been defined for '" + getTypeName() + "'");
}

void MobileUnit::setMoveTarget(Target _target, float newRange)
{
    if (auto point = _target.getPointUnlessTargetDeleted(*this->getGameOrThrow()))
    {
        float currentRangeSquared = (this->getPos() - *point).getMagnitudeSquared();
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
bool MobileUnit::isIdle()
{
    if (auto targetInfo = maybeTargetInfo)
    {
        if (optional<vector2f> p = targetInfo->target.getPointUnlessTargetDeleted(*getGameOrThrow()))
        {
            vector2f toPoint = *p - getPos();
            float distanceLeft = toPoint.getMagnitude() - targetInfo->desiredRange;
            return (distanceLeft <= EPSILON);
        }
    }
    return true;
}
optional<Target> MobileUnit::getMoveTarget()
{
    if (maybeTargetInfo)
        return maybeTargetInfo->target;
    else
        return {};
}

void MobileUnit::moveWithVelocityAndUpdateCell(vector2f velocity)
{
    lastVelocity = velocity;
    if (velocity.getMagnitudeSquared() >= EPSILON)
    {
        angle_view = velocity.getAngle();
    }
    setPosAndUpdateCell(getPos() + velocity);
}
void MobileUnit::tryMoveTowardPoint(vector2f dest, float range)
{
    vector2f toPoint = dest - getPos();
    float distanceLeft = toPoint.getMagnitude() - range;
    if (distanceLeft <= EPSILON)
    {
        return;
    }

    vector2f unitDir = toPoint.normalized();
    angle_view = unitDir.getAngle();

    if (distanceLeft <= getMaxSpeed())
    {
        desiredVelocity = unitDir * distanceLeft;
    }
    else
    {
        desiredVelocity = unitDir * getMaxSpeed();
    }
}
void MobileUnit::mobileUnitIterate()
{
    desiredVelocity = vector2f(0,0);

    if (maybeTargetInfo)
    {
        if (optional<vector2f> p = maybeTargetInfo->target.getPointUnlessTargetDeleted(*getGameOrThrow()))
        {
            float distanceSquared = (getPos() - *p).getMagnitudeSquared();
            // if we're "breaking a record" for closest to the point, set frustration to 0
            if (distanceSquared <= maybeTargetInfo->closestDistanceSquared)
            {
                maybeTargetInfo->closestDistanceSquared = distanceSquared;
                maybeTargetInfo->frustration = 0;
            }
            // otherwise, frustration mounts!
            else
            {
                maybeTargetInfo->frustration += MOBILEUNIT_FRUSTRATION_GROWTH_FACTOR;
            }

            // this factors in frustration, so the unit eventually gives up.
            // this is primarly to avoid frantic swarming when large numbers of units all go toward the same point.
            if (distanceSquared < pow(EPSILON + maybeTargetInfo->frustration, 2))
            {
                clearMoveTarget();
            }
            else
            {
                tryMoveTowardPoint(*p, maybeTargetInfo->desiredRange);
            }
        }
        else
            clearMoveTarget();
    }
    unitIterate();
}
void MobileUnit::cmdMove(vector2f pointTarget)
{
    setMoveTarget(Target(pointTarget), 0);
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



float Beacon::getRadius() const { return BEACON_RADIUS; }
unsigned char Beacon::typechar() const { return BEACON_TYPECHAR; }
string Beacon::getTypeName() const { return "Beacon"; }
coinsInt Beacon::getCost() const { return GATEWAY_COST; }
uint16_t Beacon::getMaxHealth() const { return BEACON_HEALTH; }

void Beacon::pack(vch *dest)
{
    packBuilding(dest);

    packToVch(dest, "C", (unsigned char)(state));
}
void Beacon::unpackAndMoveIter(vchIter *iter)
{
    unsigned char enumInt;
    *iter = unpackFromIter(*iter, "C", &enumInt);
    state = static_cast<State>(enumInt);
}

Beacon::Beacon(int ownerId, vector2f pos, State state)
    : Building(ownerId, GATEWAY_COST, BEACON_HEALTH, pos),
      state(state)
{}
Beacon::Beacon(vchIter *iter) : Building(iter)
{
    unpackAndMoveIter(iter);
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



float Gateway::getRadius() const {return GATEWAY_RADIUS;}
unsigned char Gateway::typechar() const { return GATEWAY_TYPECHAR; }
string Gateway::getTypeName() const { return "Gateway"; }
coinsInt Gateway::getCost() const { return GATEWAY_COST; }
uint16_t Gateway::getMaxHealth() const { return GATEWAY_HEALTH; }

void Gateway::cmdBuildUnit(unsigned char unitTypechar)
{
    if (this->isActive())
    {
        // determine position of new unit. Hacky - should replace very soon!
        float angle = (getGameOrThrow()->frame / 10.0);
        vector2f newUnitPos = this->getPos() + composeVector2f(angle, GATEWAY_RANGE / 2);
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
            if ((*point - this->getPos()).getMagnitudeSquared() > pow(GATEWAY_RANGE, 2))
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
                        if ((this->getPos() - unit->getPos()).getMagnitudeSquared() > pow(GATEWAY_RANGE, 2))
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
                    if ((this->getPos() - goldpile->getPos()).getMagnitudeSquared() > pow(GATEWAY_RANGE, 2))
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

float Gateway::buildQueueWeight()
{
    if (!isActive())
        return 10;
    else if (state == Idle)
        return 0;
    else
        return 1;
}

void Gateway::pack(vch *dest)
{
    packBuilding(dest);

    packToVch(dest, "C", (unsigned char)(state));

    if (auto targetEntity = maybeTargetEntity)
    {
        packTrue(dest);
        packEntityRef(dest, *targetEntity);
    }
    else
    {
        packFalse(dest);
    }
}
void Gateway::unpackAndMoveIter(vchIter *iter)
{
    unsigned char enumInt;
    *iter = unpackFromIter(*iter, "C", &enumInt);
    state = static_cast<State>(enumInt);

    if (unpackBoolAndMoveIter(iter))
    {
        EntityRef targetEntity;
        *iter = unpackEntityRef(*iter, &targetEntity);
        maybeTargetEntity = {targetEntity};
    }
}

Gateway::Gateway(int ownerId, vector2f pos)
    : Building(ownerId, GATEWAY_COST, GATEWAY_HEALTH, pos),
      state(Idle), inGameTransferState(NoGoldTransfer)
{}
Gateway::Gateway(vchIter *iter) : Building(iter)
{
    unpackAndMoveIter(iter);
}

void Gateway::iterate()
{
    Game *game = getGameOrThrow();

    inGameTransferState = NoGoldTransfer; // will possibly be updated in the following switch
    switch (state)
    {
        case Idle:
        {
            // search for units near enough to complete
            vector<EntityRef> nearbyEntityRefs = game->searchGrid.nearbyEntitiesSloppyIncludingEmpty(this->getPos(), GATEWAY_RANGE);

            float rangeSquared = pow(GATEWAY_RANGE, 2);
            for (unsigned int i=0; i<nearbyEntityRefs.size(); i++)
            {
                boost::shared_ptr<Entity> entity = maybeEntityRefToPtrOrNull(*game, {nearbyEntityRefs[i]});

                if (auto unit = boost::dynamic_pointer_cast<Unit, Entity>(entity))
                {
                    if (unit->ownerId == this->ownerId)
                        if (unit->getBuiltRatio() < 1)
                            if ((unit->getPos() - this->getPos()).getMagnitudeSquared() <= rangeSquared)
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
                if ((depositingToEntityPtr->getPos() - this->getPos()).getMagnitudeSquared() > pow(GATEWAY_RANGE, 2))
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
                        if (unit->getBuiltRatio() < 1)
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
                        if (maybeBuildingUnit && maybeBuildingUnit->getBuiltRatio() == 1)
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
                            inGameTransferState = Pushing;
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
                if ((this->getPos() - entity->getPos()).getMagnitudeSquared() > pow(GATEWAY_RANGE + EPSILON, 2))
                {
                    if (auto mobileUnit = boost::dynamic_pointer_cast<MobileUnit, Entity>(entity))
                    {
                        if ((!(mobileUnit->getMoveTarget())) || *(mobileUnit->getMoveTarget()->castToEntityRef()) != this->getRefOrThrow())
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
                        inGameTransferState = Pulling;
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



void Prime::pack(vch *dest)
{
    packMobileUnit(dest);

    packToVch(dest, "C", (unsigned char)(behavior));
    packToVch(dest, "C", (unsigned char)(state));

    if (auto gatherTarget = maybeGatherTargetPos)
    {
        packTrue(dest);
        packVector2f(dest, *gatherTarget);
    }
    else
    {
        packFalse(dest);
    }

    heldGold.pack(dest);
    packTypechar(dest, gonnabuildTypechar);
}
void Prime::unpackAndMoveIter(vchIter *iter)
{
    unsigned char enumInt;
    
    *iter = unpackFromIter(*iter, "C", &enumInt);
    behavior = static_cast<Behavior>(enumInt);

    *iter = unpackFromIter(*iter, "C", &enumInt);
    state = static_cast<State>(enumInt);

    if (unpackBoolAndMoveIter(iter))
    {
        vector2f targetPos;
        *iter = unpackVector2f(*iter, &targetPos);
        maybeGatherTargetPos = {targetPos};
    }
    else
    {
        maybeGatherTargetPos = {};
    }

    heldGold = Coins(iter);
    *iter = unpackTypecharFromIter(*iter, &gonnabuildTypechar);
}

Prime::Prime(int ownerId, vector2f pos)
    : MobileUnit(ownerId, PRIME_COST, PRIME_HEALTH, pos),
      heldGold(PRIME_MAX_GOLD_HELD),
      behavior(Basic), maybeGatherTargetPos({}), state(NotTransferring)
{}
Prime::Prime(vchIter *iter) : MobileUnit(iter),
                              heldGold(PRIME_MAX_GOLD_HELD)
{
    unpackAndMoveIter(iter);
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
void Prime::cmdBuild(unsigned char buildTypechar, vector2f buildPos)
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
void Prime::cmdGather(vector2f targetPos)
{
    behavior = Gather;
    state = NotTransferring;
    maybeGatherTargetPos = {targetPos};
    setMoveTarget(targetPos, 0);
}
void Prime::cmdScuttle(EntityRef targetUnit)
{
    #warning prime doesnt know how to scuttle yet
}

float Prime::getHeldGoldRatio()
{
    return ((float)this->heldGold.getInt()) / PRIME_MAX_GOLD_HELD;
}

float Prime::getRadius() const { return PRIME_RADIUS; }
float Prime::getMaxSpeed() const { return PRIME_SPEED; }
float Prime::getRange() const { return PRIME_TRANSFER_RANGE; }
coinsInt Prime::getCost() const { return PRIME_COST; }
uint16_t Prime::getMaxHealth() const { return PRIME_HEALTH; }

unsigned char Prime::typechar() const { return PRIME_TYPECHAR; }
string Prime::getTypeName() const { return "Prime"; }

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
                if (getHeldGoldRatio() == 1)
                {
                    setStateToReturnGoldOrResetBehavior();
                }

                switch (state)
                {
                    case NotTransferring:
                    {
                        // in theory this should already be set, but in some cases doesn't seem to be
                        setMoveTarget(*gatherTargetPos, 0);
                        
                        // scan for any GoldPile and choose the closest
                        boost::shared_ptr<GoldPile> bestTarget;
                        float bestTargetDistanceSquared;

                        auto entitiesInSight = game->entitiesWithinCircle(getPos(), PRIME_SIGHT_RANGE);
                        for (unsigned int i=0; i<entitiesInSight.size(); i++)
                        {
                            if (auto goldPile = boost::dynamic_pointer_cast<GoldPile, Entity>(entitiesInSight[i]))
                            {
                                float distanceSquared = (this->getPos() - goldPile->getPos()).getMagnitudeSquared();
                                if (!bestTarget || distanceSquared < bestTargetDistanceSquared)
                                {
                                    bestTarget = goldPile;
                                    bestTargetDistanceSquared = distanceSquared;
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
                        else if ((*gatherTargetPos - this->getPos()).getMagnitudeSquared() < EPSILON)
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
                        if (auto moveTarget = getMoveTarget())
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
                            setMoveTarget(*gatherTargetPos, 0);
                            break;
                        }
                        
                    }
                    break;
                    case PutdownGold:
                    {
                        if (heldGold.getInt() == 0)
                        {
                            state = NotTransferring;
                            setMoveTarget(*gatherTargetPos, 0);
                            break;
                        }
                        // as long as Gateawy (in moveTarget) is still valid, just continue til heldGold == 0
                        bool stillMovingTowardGateway = false; // until proven otherwise
                        if (auto moveTarget = getMoveTarget())
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

    goldTransferState = NoGoldTransfer;
    switch (state)
    {
    case NotTransferring:
        break;
    case PickupGold:
        if (auto target = getMoveTarget())
        {
            if (boost::shared_ptr<Entity> e = target->castToEntityPtr(*game))
            {
                if ((e->getPos() - this->getPos()).getMagnitude() <= PRIME_TRANSFER_RANGE + EPSILON)
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
                            goldTransferState = Pulling;
                    }
                }
            }
        }
        break;
    case PutdownGold:
        if (auto target = getMoveTarget())
        if (optional<vector2f> point = target->getPointUnlessTargetDeleted(*game))
        {
            if ((*point - getPos()).getMagnitude() <= PRIME_TRANSFER_RANGE + EPSILON)
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
                        if (unit->getBuiltRatio() < 1)
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
                        goldTransferState = Pushing;
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
        if (auto target = getMoveTarget())
        {
            if (optional<vector2f> point = target->castToPoint())
            {
                if ((*point - getPos()).getMagnitude() <= PRIME_TRANSFER_RANGE + EPSILON)
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
                    if (building->getBuiltRatio() < 1)
                    {
                        coinsInt builtAmount = building->build(PRIME_PUTDOWN_RATE, &this->heldGold);
                        if (builtAmount > 0)
                        {
                            goldTransferState = Pushing;
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
    mobileUnitIterate();
}

void Prime::setStateToReturnGoldOrResetBehavior()
{
    Game *game = this->getGameOrThrow();

    // find nearest gateway and bring gold to it
    boost::shared_ptr<Gateway> bestChoice;
    float bestChoiceDistanceSquared;
    for (unsigned int i=0; i<game->entities.size(); i++)
    {
        if (auto gateway = boost::dynamic_pointer_cast<Gateway, Entity>(game->entities[i]))
        {
            if (getAllianceType(this->ownerId, gateway) == Owned)
            {
                float distanceSquared = (this->getPos() - gateway->getPos()).getMagnitudeSquared();
                if (!bestChoice || distanceSquared < bestChoiceDistanceSquared)
                {
                    bestChoice = gateway;
                    bestChoiceDistanceSquared = distanceSquared;
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

void Prime::onMoveCmd(vector2f moveTo)
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



void Fighter::pack(vch *dest)
{
    packMobileUnit(dest);

    packToVch(dest, "C", (unsigned char)(state));
    
    if (maybeAttackingGeneralTarget)
    {
        packTrue(dest);
        maybeAttackingGeneralTarget->pack(dest);
    }
    else
    {
        packFalse(dest);
    }

    packToVch(dest, "H", shootCooldown);
}
void Fighter::unpackAndMoveIter(vchIter *iter)
{
    unsigned char enumInt;
    *iter = unpackFromIter(*iter, "C", &enumInt);
    state = static_cast<State>(enumInt);

    if (unpackBoolAndMoveIter(iter))
    {
        maybeAttackingGeneralTarget = Target(iter);
    }

    *iter = unpackFromIter(*iter, "H", &shootCooldown);
}

Fighter::Fighter(int ownerId, vector2f pos)
    : MobileUnit(ownerId, FIGHTER_COST, FIGHTER_HEALTH, pos),
      state(NotAttacking), maybeAttackingGeneralTarget({}), shootCooldown(0), animateShot(None), lastShot(None)
{}
Fighter::Fighter(vchIter *iter)
    : MobileUnit(iter)
{
    unpackAndMoveIter(iter);
}

void Fighter::cmdAttack(Target target)
{
    if (auto ref = target.castToEntityRef())
    {
        state = AttackingSpecific;
    }
    else if (auto point = target.castToPoint())
    {
        state = AttackingGeneral;
        maybeAttackingGeneralTarget = point;
    }
    setMoveTarget(target, FIGHTER_SHOT_RANGE);
}
void Fighter::iterate()
{
    Game *game = getGameOrThrow();

    animateShot = None;
    if (shootCooldown > 0)
        shootCooldown --;

    switch (state)
    {
        case NotAttacking:
        {
            if (MobileUnit::isIdle())
            {
                auto entitiesInSight = game->entitiesWithinCircle(getPos(), FIGHTER_SIGHT_RANGE);

                boost::shared_ptr<Entity> closestValidTarget;
                float closestDistanceSquared;
                for (unsigned int i=0; i<entitiesInSight.size(); i++)
                {
                    auto entity = entitiesInSight[i];
                    if (getAllianceType(this->ownerId, entity) == Foreign)
                    {
                        float distanceSquared = (this->getPos() - entity->getPos()).getMagnitudeSquared();
                        if (!closestValidTarget || distanceSquared < closestDistanceSquared)
                        {
                            closestValidTarget = entity;
                            closestDistanceSquared = distanceSquared;
                        }
                    }
                }

                if (closestValidTarget)
                {
                    state = AttackingGeneral;
                    maybeAttackingGeneralTarget = Target(closestValidTarget->getRefOrThrow());
                }
            }
        }
        break;
        case AttackingGeneral:
        {
            if (auto attackingGeneralTarget = maybeAttackingGeneralTarget)
            {
                // first make sure we're not going after some dead unit
                if (!attackingGeneralTarget->isStillValid(*game))
                {
                    state = NotAttacking;
                    attackingGeneralTarget = {};
                    break;
                }

                // same for moveTarget, but don't return to idle; just clear the var so the next if catches
                if (auto moveTarget = getMoveTarget())
                {
                    if (!moveTarget->isStillValid(*game))
                    {
                        MobileUnit::clearMoveTarget();
                    }
                }

                // if we're not moving toward the target, do so
                if (!getMoveTarget())
                {
                    if (attackingGeneralTarget->type == Target::PointTarget)
                    {
                        setMoveTarget(*attackingGeneralTarget, 0);
                    }
                    else
                    {
                        setMoveTarget(*attackingGeneralTarget, FIGHTER_SHOT_RANGE);
                    }
                }

                // if the target is a point and we've arrived, go back to Idle
                if (auto point = attackingGeneralTarget->castToPoint())
                {
                    if ((*point - this->getPos()).getMagnitudeSquared() < EPSILON)
                    {
                        state = NotAttacking;
                        attackingGeneralTarget = {};
                        break;
                    }
                }

                // at this point we know:
                    // the target is either a live unit or a position we haven't arrived at
                    // we are moving toward the target or staying within range
                
                // we must also keep in mind that moveTarget may be different than attackingGeneralTarget
                    // (the former changes more often, the latter is the Fighter's "higher order" command)
                    // the moveTarget is what the Figher is attacking "right now"
                
                // we will search for good options for attack
                boost::shared_ptr<Unit> bestTarget;
                float bestTargetPriority;
                bool alreadyHadTarget = false;
                if (auto unit = boost::dynamic_pointer_cast<Unit,Entity>(getMoveTarget()->castToEntityPtr(*game)))
                {
                    bestTarget = unit;
                    bestTargetPriority = this->calcAttackPriority(unit);
                    alreadyHadTarget = true; // might be overridden by a higher priority unit nearby, but is sticky in the face of ties
                }

                auto entitiesInSight = game->entitiesWithinCircle(getPos(), FIGHTER_SIGHT_RANGE);
                for (unsigned int i=0; i<entitiesInSight.size(); i++)
                {
                    if (auto unit = boost::dynamic_pointer_cast<Unit, Entity>(entitiesInSight[i]))
                    {
                        if (getAllianceType(this->ownerId, entitiesInSight[i]) == Foreign)
                        {
                            bool thisIsBetterTarget = false; // until proven otherwise

                            float priority = this->calcAttackPriority(unit);

                            // if there is not yet a bestTarget, or if the priority is higher
                            if ((!bestTarget) || priority > bestTargetPriority)
                            {
                                thisIsBetterTarget = true;
                            }

                            // if the priority matches and we didn't already have a target before this frame...
                            else if (priority == bestTargetPriority && !alreadyHadTarget)
                            {
                                // compare distances
                                float currentDistanceSquared = (this->getPos() - bestTarget->getPos()).getMagnitudeSquared();
                                float distanceSquared = (this->getPos() - unit->getPos()).getMagnitudeSquared();
                                if (distanceSquared < currentDistanceSquared)
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
                    setMoveTarget(Target(bestTarget), FIGHTER_SHOT_RANGE);
                    tryShootAt(bestTarget);
                }
            }
            else
            {
                cout << "state is attackingGeneral, but there is no attackingGeneralTarget..." << endl;
                state = NotAttacking;
            }
        }
        break;
        case AttackingSpecific:
            if (auto target = getMoveTarget())
            {
                bool returnToIdle = false;
                if (auto targetEntity = target->castToEntityPtr(*game)) // will return false if unit died (pointer will be empty)
                {
                    if (auto targetUnit = boost::dynamic_pointer_cast<Unit, Entity>(targetEntity))
                    {
                        tryShootAt(targetUnit);
                    }
                    else
                        returnToIdle = true;
                }
                else
                    returnToIdle = true;

                if (returnToIdle)
                {
                    state = NotAttacking;
                }
            }
            break;
    }
    mobileUnitIterate();
}
void Fighter::onMoveCmd(vector2f moveTo)
{
    state = NotAttacking;
}

void Fighter::tryShootAt(boost::shared_ptr<Unit> targetUnit)
{
    vector2f toTarget = (targetUnit->getPos() - this->getPos());
    angle_view = toTarget.getAngle();
    if (toTarget.getMagnitude() <= FIGHTER_SHOT_RANGE + EPSILON)
    {
        if (shootCooldown == 0)
        {
            shootAt(targetUnit);
            animateShot = (lastShot != Left) ? Left : Right;
            lastShot = animateShot;
        }
    }
}
float Fighter::calcAttackPriority(boost::shared_ptr<Unit> foreignUnit)
{
    if (auto building = boost::dynamic_pointer_cast<Building, Unit>(foreignUnit))
    {
        return 1;
    }
    else if (auto prime = boost::dynamic_pointer_cast<Prime, Unit>(foreignUnit))
    {
        return 2;
    }
    else if (auto fighter = boost::dynamic_pointer_cast<Fighter, Unit>(foreignUnit))
    {
        float baseFighterPriority = 3;
        if (auto otherFighterTarget = fighter->getMoveTarget())
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
        return 0;
    }
}
void Fighter::shootAt(boost::shared_ptr<Unit> unit)
{
    shootCooldown = FIGHTER_SHOT_COOLDOWN;
    unit->takeHit(FIGHTER_DAMAGE);
}

float Fighter::getRadius() const { return FIGHTER_RADIUS; }
float Fighter::getMaxSpeed() const { return FIGHTER_SPEED; }
float Fighter::getRange() const { return FIGHTER_SHOT_RANGE; }
coinsInt Fighter::getCost() const { return FIGHTER_COST; }
uint16_t Fighter::getMaxHealth() const { return FIGHTER_HEALTH; }

unsigned char Fighter::typechar() const { return FIGHTER_TYPECHAR; }
string Fighter::getTypename() const { return "Fighter"; }



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



boost::shared_ptr<Entity> unpackFullEntityAndMoveIter(vchIter *iter, unsigned char typechar)
{
    switch (typechar)
    {
    case NULL_TYPECHAR:
        return boost::shared_ptr<Entity>();
        break;
    case GOLDPILE_TYPECHAR:
        return boost::shared_ptr<Entity>(new GoldPile(iter));
        break;
    case BEACON_TYPECHAR:
        return boost::shared_ptr<Entity>(new Beacon(iter));
        break;
    case GATEWAY_TYPECHAR:
        return boost::shared_ptr<Entity>(new Gateway(iter));
        break;
    case PRIME_TYPECHAR:
        return boost::shared_ptr<Entity>(new Prime(iter));
        break;
    case FIGHTER_TYPECHAR:
        return boost::shared_ptr<Entity>(new Fighter(iter));
        break;
    }
    throw runtime_error("Trying to unpack an unrecognized entity");
}

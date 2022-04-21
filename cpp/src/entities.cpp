#include <boost/shared_ptr.hpp>
#include "engine.h"
#include "entities.h"




// ----------------------
// ----------------------
// ----------------------
// ----------------------
// ----------------------
// ----------------------
// ----------------------
// ----------------------
// ----------------------
// ------- TARGET -------
// ----------------------
// ----------------------
// ----------------------
// ----------------------
// ----------------------
// ----------------------
// ----------------------
// ----------------------
// ----------------------



void Target::pack(vch *dest)
{
    packToVch(dest, "C", (unsigned char)(type));
    if (type == PointTarget)
    {
        packVector2f(dest, pointTarget);
    }
    else
    {
        packEntityRef(dest, entityTarget);
    }
}
void Target::unpackAndMoveIter(vchIter *iter)
{
    unsigned char enumInt;
    *iter = unpackFromIter(*iter, "C", &enumInt);
    type = static_cast<Type>(enumInt);

    if (type == PointTarget)
    {
        *iter = unpackVector2f(*iter, &pointTarget);
    }
    else
    {
        *iter = unpackEntityRef(*iter, &entityTarget);
    }
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




// ----------------------
// ----------------------
// ----------------------
// ----------------------
// ----------------------
// ----------------------
// ----------------------
// ----------------------
// ----------------------
// ------- MISC ---------
// ----------------------
// ----------------------
// ----------------------
// ----------------------
// ----------------------
// ----------------------
// ----------------------
// ----------------------
// ----------------------



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




// ----------------------
// ----------------------
// ----------------------
// ----------------------
// ----------------------
// ----------------------
// ----------------------
// ----------------------
// ----------------------
// ------- ENTITY -------
// ----------------------
// ----------------------
// ----------------------
// ----------------------
// ----------------------
// ----------------------
// ----------------------
// ----------------------
// ----------------------


vector2f Entity::getPos()
{
    return pos;
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
unsigned char Entity::typechar()
{
    throw runtime_error("typechar() has not been defined for " + getTypeName() + ".");
}
string Entity::getTypeName()
{
    throw runtime_error("getTypeName() has not been defined for this unit.");
}

bool Entity::collidesWithPoint(vector2f point)
{
    return (pos - point).getMagnitude() <= ENTITY_COLLIDE_RADIUS;
}

void Entity::go()
{
    throw runtime_error("go() has not been defined for " + getTypeName() + ".");
}
sf::Color Entity::getTeamOrPrimaryColor()
{
    throw runtime_error("getTeamOrPrimaryColor() has not been defined for " + getTypeName() + ".");
}
void Entity::pack(vch *dest)
{
    throw runtime_error("pack() has not been defined for " + getTypeName() + ".");
}
void Entity::unpackAndMoveIter(vchIter *iter, Game &game)
{
    throw runtime_error("unpackMoveIter() has not been defined for " + getTypeName() + ".");
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



// ----------------------
// ----------------------
// ----------------------
// ----------------------
// ----------------------
// ----------------------
// ----------------------
// ----------------------
// ----------------------
// ------- GOLDPILE -----
// ----------------------
// ----------------------
// ----------------------
// ----------------------
// ----------------------
// ----------------------
// ----------------------
// ----------------------
// ----------------------



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

unsigned char GoldPile::typechar() { return GOLDPILE_TYPECHAR; }
string GoldPile::getTypeName() { return "GoldPile"; }
void GoldPile::go()
{
    if (gold.getInt() == 0)
        die();
}



// ----------------------
// ----------------------
// ----------------------
// ----------------------
// ----------------------
// ----------------------
// ----------------------
// ----------------------
// ----------------------
// ------- UNIT ---------
// ----------------------
// ----------------------
// ----------------------
// ----------------------
// ----------------------
// ----------------------
// ----------------------
// ----------------------
// ----------------------



vector<Coins*> Unit::getDroppableCoins()
{
    return vector<Coins*>{&goldInvested};
}
coinsInt Unit::getCost()
{
    throw runtime_error("getCost() has not been defined for " + getTypeName() + ".");
}
uint16_t Unit::getMaxHealth()
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
        return playerAddressToColor(regInfo->game->playerIdToAddress(ownerId));
    else
        return sf::Color(150, 150, 150);
}
void Unit::unitGo() {}
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



// ----------------------
// ----------------------
// ----------------------
// ----------------------
// ----------------------
// ----------------------
// ----------------------
// ----------------------
// ----------------------
// ------- BUILDING -----
// ----------------------
// ----------------------
// ----------------------
// ----------------------
// ----------------------
// ----------------------
// ----------------------
// ----------------------
// ----------------------



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

void Building::buildingGo()
{
    unitGo();
}



// ----------------------
// ----------------------
// ----------------------
// ----------------------
// ----------------------
// ----------------------
// ----------------------
// ----------------------
// ----------------------
// ------- MOBILEUNIT ---
// ----------------------
// ----------------------
// ----------------------
// ----------------------
// ----------------------
// ----------------------
// ----------------------
// ----------------------
// ----------------------



void MobileUnit::packMobileUnit(vch *dest)
{
    packUnit(dest);
    if (auto targetAndRange = maybeTargetAndRange)
    {
        packTrue(dest);

        targetAndRange->first.pack(dest);
        packToVch(dest, "f", targetAndRange->second);
    }
    else
    {
        packFalse(dest);
    }
}
void MobileUnit::unpackMobileUnitAndMoveIter(vchIter *iter)
{
    if (unpackBoolAndMoveIter(iter))
    {
        Target target(iter);

        float range;
        *iter = unpackFromIter(*iter, "f", &range);

        maybeTargetAndRange = {pair(target, range)};
    }
}

MobileUnit::MobileUnit(int ownerId, coinsInt totalCost, uint16_t health, vector2f pos)
    : Unit(ownerId, totalCost, health, pos), maybeTargetAndRange({}), angle_view(0)
{}
MobileUnit::MobileUnit(vchIter *iter) : Unit(iter),
                                        angle_view(0)
{
    unpackMobileUnitAndMoveIter(iter);
}

float MobileUnit::getSpeed()
{
    throw runtime_error("getSpeed has not been defined for '" + getTypeName() + "'");
}
float MobileUnit::getRange()
{
    throw runtime_error("getRange has not been defined for '" + getTypeName() + "'");
}
void MobileUnit::onMoveCmd(vector2f moveTo)
{
    throw runtime_error("onMoveCmd() has not been defined for '" + getTypeName() + "'");
}

void MobileUnit::setTarget(Target _target, float newRange)
{
    maybeTargetAndRange = {pair(_target, newRange)};
}
void MobileUnit::clearTarget()
{
    maybeTargetAndRange = {};
}
bool MobileUnit::isIdle()
{
    if (auto targetAndRange = maybeTargetAndRange)
    {
        if (optional<vector2f> p = targetAndRange->first.getPointUnlessTargetDeleted(*getGameOrThrow()))
        {
            vector2f toPoint = *p - getPos();
            float distanceLeft = toPoint.getMagnitude() - targetAndRange->second;
            return (distanceLeft <= DISTANCE_TOL);
        }
    }
    return true;
}
optional<Target> MobileUnit::getTarget()
{
    if (maybeTargetAndRange)
        return maybeTargetAndRange->first;
    else
        return {};
}

void MobileUnit::addToPosAndUpdateCell(vector2f toAdd)
{
    setPosAndUpdateCell(getPos() + toAdd);
}
void MobileUnit::moveTowardPoint(vector2f dest, float range)
{
    vector2f toPoint = dest - getPos();
    float distanceLeft = toPoint.getMagnitude() - range;
    if (distanceLeft <= DISTANCE_TOL)
    {
        return;
    }

    vector2f unitDir = toPoint.normalized();
    angle_view = unitDir.getAngle();

    if (distanceLeft <= getSpeed())
    {
        addToPosAndUpdateCell(unitDir * distanceLeft);
    }
    else
    {
        addToPosAndUpdateCell(unitDir * getSpeed());
    }
}
void MobileUnit::mobileUnitGo()
{
    if (auto targetAndRange = maybeTargetAndRange)
    {
        if (optional<vector2f> p = targetAndRange->first.getPointUnlessTargetDeleted(*getGameOrThrow()))
            moveTowardPoint(*p, targetAndRange->second);
        else
            setTarget(Target(getPos()), 0);
    }
    unitGo();
}
void MobileUnit::cmdMove(vector2f pointTarget)
{
    setTarget(Target(pointTarget), 0);
    onMoveCmd(pointTarget);
}



// ----------------------
// ----------------------
// ----------------------
// ----------------------
// ----------------------
// ----------------------
// ----------------------
// ----------------------
// ----------------------
// ------- BEACON -------
// ----------------------
// ----------------------
// ----------------------
// ----------------------
// ----------------------
// ----------------------
// ----------------------
// ----------------------
// ----------------------



unsigned char Beacon::typechar() { return BEACON_TYPECHAR; }
string Beacon::getTypeName() { return "Beacon"; }
coinsInt Beacon::getCost() { return BEACON_COST; }
uint16_t Beacon::getMaxHealth() { return BEACON_HEALTH; }

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
    : Building(ownerId, BEACON_COST, BEACON_HEALTH, pos),
      state(state)
{}
Beacon::Beacon(vchIter *iter) : Building(iter)
{
    unpackAndMoveIter(iter);
}

void Beacon::go()
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
                game->registerNewEntity(gateway);
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



// ----------------------
// ----------------------
// ----------------------
// ----------------------
// ----------------------
// ----------------------
// ----------------------
// ----------------------
// ----------------------
// ------- GATEWAY ------
// ----------------------
// ----------------------
// ----------------------
// ----------------------
// ----------------------
// ----------------------
// ----------------------
// ----------------------
// ----------------------



unsigned char Gateway::typechar() { return GATEWAY_TYPECHAR; }
string Gateway::getTypeName() { return "Gateway"; }
coinsInt Gateway::getCost() { return GATEWAY_COST; }
uint16_t Gateway::getMaxHealth() { return GATEWAY_HEALTH; }

void Gateway::cmdBuildUnit(unsigned char unitTypechar)
{
    if (this->isActive())
    {
        vector2f newUnitPos = this->getPos() + randomVectorWithMagnitudeRange(20, GATEWAY_RANGE);
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
            getGameOrThrow()->registerNewEntity(littleBabyUnitAwwwwSoCute);
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
            getGameOrThrow()->registerNewEntity(goldpile);
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
            game->registerNewEntity(beacon);
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
                                mobileUnit->setTarget(this->getRefOrThrow(), GATEWAY_RANGE);
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

void Gateway::go()
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
                if ((this->getPos() - entity->getPos()).getMagnitudeSquared() > pow(GATEWAY_RANGE + DISTANCE_TOL, 2))
                {
                    if (auto mobileUnit = boost::dynamic_pointer_cast<MobileUnit, Entity>(entity))
                    {
                        if ((!(mobileUnit->getTarget())) || *(mobileUnit->getTarget()->castToEntityRef()) != this->getRefOrThrow())
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
                        amountScuttled = goldPile->gold.transferUpTo(SCUTTLE_RATE, &game->players[this->ownerId].credit);
                    }
                    else if (auto unit = boost::dynamic_pointer_cast<Unit, Entity>(entity))
                    {
                        amountScuttled = unit->unbuild(SCUTTLE_RATE, &game->players[this->ownerId].credit);
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



// ----------------------
// ----------------------
// ----------------------
// ----------------------
// ----------------------
// ----------------------
// ----------------------
// ----------------------
// ----------------------
// ------- PRIME --------
// ----------------------
// ----------------------
// ----------------------
// ----------------------
// ----------------------
// ----------------------
// ----------------------
// ----------------------
// ----------------------



void Prime::pack(vch *dest)
{
    packMobileUnit(dest);

    packToVch(dest, "C", (unsigned char)(state));

    heldGold.pack(dest);
    packTypechar(dest, gonnabuildTypechar);
}
void Prime::unpackAndMoveIter(vchIter *iter)
{
    unsigned char enumInt;
    *iter = unpackFromIter(*iter, "C", &enumInt);
    state = static_cast<State>(enumInt);

    heldGold = Coins(iter);
    *iter = unpackTypecharFromIter(*iter, &gonnabuildTypechar);
}

Prime::Prime(int ownerId, vector2f pos)
    : MobileUnit(ownerId, PRIME_COST, PRIME_HEALTH, pos),
      heldGold(PRIME_MAX_GOLD_HELD),
      state(Idle)
{}
Prime::Prime(vchIter *iter) : MobileUnit(iter),
                              heldGold(PRIME_MAX_GOLD_HELD)
{
    unpackAndMoveIter(iter);
}

void Prime::cmdPickup(Target _target)
{
    state = PickupGold;

    setTarget(_target, PRIME_RANGE);
}
void Prime::cmdPutdown(Target _target)
{
    state = PutdownGold;

    setTarget(_target, PRIME_RANGE);
}
void Prime::cmdBuild(unsigned char buildTypechar, vector2f buildPos)
{
    state = Build;
    gonnabuildTypechar = buildTypechar;

    setTarget(buildPos, PRIME_RANGE);
}
void Prime::cmdResumeBuilding(EntityRef targetUnit)
{
    state = Build;

    setTarget(Target(targetUnit), PRIME_RANGE);
}
void Prime::cmdScuttle(EntityRef targetUnit)
{
    #warning prime doesnt know how to scuttle yet
}

float Prime::getSpeed() { return PRIME_SPEED; }
float Prime::getRange() { return PRIME_RANGE; }
coinsInt Prime::getCost() { return PRIME_COST; }
uint16_t Prime::getMaxHealth() { return PRIME_HEALTH; }

unsigned char Prime::typechar() { return PRIME_TYPECHAR; }
string Prime::getTypeName() { return "Prime"; }

void Prime::go()
{
    Game *game = getGameOrThrow();

    goldTransferState = NoGoldTransfer;
    switch (state)
    {
    case Idle:
        break;
    case PickupGold:
        if (auto target = getTarget())
        {
            if (boost::shared_ptr<Entity> e = target->castToEntityPtr(*game))
            {
                if ((e->getPos() - this->getPos()).getMagnitude() <= PRIME_RANGE + DISTANCE_TOL)
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
                            state = Idle;
                        else
                            goldTransferState = Pulling;
                    }
                }
            }
        }
        break;
    case PutdownGold:
        if (auto target = getTarget())
        if (optional<vector2f> point = target->getPointUnlessTargetDeleted(*game))
        {
            if ((*point - getPos()).getMagnitude() <= PRIME_RANGE + DISTANCE_TOL)
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
                            state = Idle;
                        }
                    }
                    else {
                        cout << "not sure how to execute a PutdownGold cmd for a unit other than a gateway or goldpile" << endl;
                        state = Idle;
                    }
                }
                else
                {
                    // must create goldPile
                    boost::shared_ptr<GoldPile> gp(new GoldPile(*point));
                    game->registerNewEntity(gp);
                    coinsToPushTo = &gp->gold;
                    setTarget(Target(gp->getRefOrThrow()), PRIME_RANGE);
                }

                if (coinsToPushTo)
                {
                    coinsInt amountPutDown = this->heldGold.transferUpTo(PRIME_PUTDOWN_RATE, (*coinsToPushTo));
                    if (amountPutDown == 0 && stopOnTransferZero)
                    {
                        state = Idle;
                    }
                    if (amountPutDown != 0)
                    {
                        goldTransferState = Pushing;
                    }
                }
                else
                {
                    cout << "I'm confused about how to execute this putdownGold cmd." << endl;
                    state = Idle;
                }
            }
        }
        break;
    case Build:
        if (auto target = getTarget())
        {
            if (optional<vector2f> point = target->castToPoint())
            {
                if ((*point - getPos()).getMagnitude() <= PRIME_RANGE + DISTANCE_TOL)
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
                        game->registerNewEntity(buildingToBuild);
                        setTarget(Target(buildingToBuild), PRIME_RANGE);
                    }
                    else
                    {
                        cout << "Prime refuses to build for that typechar!" << endl;
                        state = Idle;
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
                        state = Idle;
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
                state = Idle;
            }
        }
        break;
    }
    mobileUnitGo();
}

void Prime::onMoveCmd(vector2f moveTo)
{
    state = Idle;
}

vector<Coins*> Prime::getDroppableCoins()
{
    return vector<Coins*>{&goldInvested, &heldGold};
}



// ----------------------
// ----------------------
// ----------------------
// ----------------------
// ----------------------
// ----------------------
// ----------------------
// ----------------------
// ----------------------
// ------- FIGHTER ------
// ----------------------
// ----------------------
// ----------------------
// ----------------------
// ----------------------
// ----------------------
// ----------------------
// ----------------------
// ----------------------



void Fighter::pack(vch *dest)
{
    packMobileUnit(dest);

    packToVch(dest, "C", (unsigned char)(state));
    packToVch(dest, "H", shootCooldown);
}
void Fighter::unpackAndMoveIter(vchIter *iter)
{
    unsigned char enumInt;
    *iter = unpackFromIter(*iter, "C", &enumInt);
    state = static_cast<State>(enumInt);

    *iter = unpackFromIter(*iter, "H", &shootCooldown);
}

Fighter::Fighter(int ownerId, vector2f pos)
    : MobileUnit(ownerId, FIGHTER_COST, FIGHTER_HEALTH, pos),
      state(NotAttacking), shootCooldown(0), animateShot(None), lastShot(None)
{}
Fighter::Fighter(vchIter *iter)
    : MobileUnit(iter)
{
    unpackAndMoveIter(iter);
}

void Fighter::cmdAttack(EntityRef ref)
{
    state = AttackingUnit;
    setTarget(Target(ref), FIGHTER_SHOT_RANGE);
}
void Fighter::go()
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
                auto entitiesWithinRadius = game->entitiesWithinRadius(getPos(), FIGHTER_SIGHT_RANGE);

                boost::shared_ptr<Entity> closestValidTarget;
                float closestDistanceSquared;
                for (unsigned int i=0; i<entitiesWithinRadius.size(); i++)
                {
                    auto entity = entitiesWithinRadius[i];
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
                    state = AttackingUnit;
                    setTarget(Target(closestValidTarget->getRefOrThrow()), FIGHTER_SHOT_RANGE);
                }
            }
        }
        break;
        case AttackingUnit:
            if (auto target = getTarget())
            {
                bool returnToIdle = false;
                if (auto targetEntity = target->castToEntityPtr(*game)) // will return false if unit died (pointer will be empty)
                {
                    if (auto targetUnit = boost::dynamic_pointer_cast<Unit, Entity>(targetEntity))
                    {
                        vector2f toTarget = (targetUnit->getPos() - this->getPos());
                        angle_view = toTarget.getAngle();
                        if (toTarget.getMagnitude() <= FIGHTER_SHOT_RANGE + DISTANCE_TOL)
                        {
                            if (shootCooldown == 0)
                            {
                                shootAt(targetUnit);
                                animateShot = (lastShot != Left) ? Left : Right;
                                lastShot = animateShot;
                            }
                        }
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
    mobileUnitGo();
}
void Fighter::onMoveCmd(vector2f moveTo)
{
    state = NotAttacking;
}

void Fighter::shootAt(boost::shared_ptr<Unit> unit)
{
    shootCooldown = FIGHTER_SHOT_COOLDOWN;
    unit->takeHit(FIGHTER_DAMAGE);
}

float Fighter::getSpeed() { return FIGHTER_SPEED; }
float Fighter::getRange() { return FIGHTER_SHOT_RANGE; }
coinsInt Fighter::getCost() { return FIGHTER_COST; }
uint16_t Fighter::getMaxHealth() { return FIGHTER_HEALTH; }

unsigned char Fighter::typechar() { return FIGHTER_TYPECHAR; }
string Fighter::getTypename() { return "Fighter"; }



// ----------------------
// ----------------------
// ----------------------
// ----------------------
// ----------------------
// ----------------------
// ----------------------
// ----------------------
// ----------------------
// ----- UNPACKENTITY ---
// ----------------------
// ----------------------
// ----------------------
// ----------------------
// ----------------------
// ----------------------
// ----------------------
// ----------------------
// ----------------------



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

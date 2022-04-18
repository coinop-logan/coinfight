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
    entityTarget = NULL_ENTITYREF;
    pointTarget = _pointTarget;
}
Target::Target(EntityRef _entityTarget)
{
    type = EntityTarget;
    pointTarget = vector2f(0, 0);
    entityTarget = _entityTarget;
}
Target::Target(boost::shared_ptr<Entity> entity)
    : Target(entity->ref) {}

optional<vector2f> Target::getPointUnlessTargetDeleted(const Game &game)
{
    if (type == PointTarget)
        return {pointTarget};
    else
        if (boost::shared_ptr<Entity> e = entityRefToPtrOrNull(game, entityTarget))
            return {e->pos};
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
        return entityRefToPtrOrNull(game, *eRef);
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




vector<EntityRef> entityPtrsToRefs(vector<boost::shared_ptr<Entity>> ptrs)
{
    vector<EntityRef> refs;
    for (unsigned int i = 0; i < ptrs.size(); i++)
    {
        refs.push_back(ptrs[i]->ref);
    }
    return refs;
}

vector<EntityRef> entityPtrsToRefs(vector<boost::shared_ptr<Unit>> unitPtrs)
{
    vector<EntityRef> refs;
    for (unsigned int i=0; i<unitPtrs.size(); i++)
    {
        refs.push_back(unitPtrs[i]->ref);
    }
    return refs;
}

boost::shared_ptr<Entity> entityRefToPtrOrNull(const Game& game, EntityRef ref)
{
    if (ref == NULL_ENTITYREF)
    {
        return boost::shared_ptr<Entity>();
    }
    else
    {
        return game.entities[ref - 1];
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
            return Enemy;
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
Entity::Entity(Game *game, EntityRef ref, vector2f pos) : game(game),
                                                          dead(false),
                                                          ref(ref),
                                                          pos(pos)
{}
Entity::Entity(Game *game, EntityRef ref, vchIter *iter) : game(game),
                                                           ref(ref)
{
    unpackEntityAndMoveIter(iter);
}
vector2f Entity::getPos()
{
    return pos;
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

GoldPile::GoldPile(Game *game, EntityRef ref, vector2f pos) : Entity(game, ref, pos),
                                                              gold(MAX_COINS)
{}
GoldPile::GoldPile(Game *game, EntityRef ref, vchIter *iter) : Entity(game, ref, iter),
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

Unit::Unit(Game *game, EntityRef ref, int ownerId, coinsInt totalCost, uint16_t health, vector2f pos)
    : Entity(game, ref, pos), health(health), ownerId(ownerId), goldInvested(totalCost) {}

Unit::Unit(Game *game, EntityRef ref, vchIter *iter) : Entity(game, ref, iter),
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
    else
        return playerAddressToColor(game->playerIdToAddress(ownerId));
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

Building::Building(Game *game, EntityRef ref, int ownerId, coinsInt totalCost, uint16_t health, vector2f pos)
    : Unit(game, ref, ownerId, totalCost, health, pos) {}
Building::Building(Game *game, EntityRef ref, vchIter *iter) : Unit(game, ref, iter)
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
    target.pack(dest);
    packToVch(dest, "f", targetRange);
}
void MobileUnit::unpackMobileUnitAndMoveIter(vchIter *iter)
{
    target = Target(iter);
    *iter = unpackFromIter(*iter, "f", &targetRange);
}

MobileUnit::MobileUnit(Game *game, EntityRef ref, int ownerId, coinsInt totalCost, uint16_t health, vector2f pos)
    : Unit(game, ref, ownerId, totalCost, health, pos), target(NULL_ENTITYREF), angle_view(0)
{
    targetRange = 0;
    setTarget(Target(pos), 0);
}
MobileUnit::MobileUnit(Game *game, EntityRef ref, vchIter *iter) : Unit(game, ref, iter),
                                                                  target(NULL_ENTITYREF),
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
    target = _target;
    targetRange = newRange;
}
Target MobileUnit::getTarget()
{
    return target;
}

void MobileUnit::moveTowardPoint(vector2f dest, float range)
{
    vector2f toPoint = dest - pos;
    float distanceLeft = toPoint.getMagnitude() - range;
    if (distanceLeft <= 0)
    {
        return;
    }

    vector2f unitDir = toPoint.normalized();
    angle_view = unitDir.getAngle();

    if (distanceLeft <= getSpeed())
    {
        pos += unitDir * distanceLeft;
    }
    else
    {
        pos += unitDir * getSpeed();
    }
}
void MobileUnit::mobileUnitGo()
{
    if (optional<vector2f> p = target.getPointUnlessTargetDeleted(*game))
        moveTowardPoint(*p, targetRange);
    else
        setTarget(Target(pos), 0);
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

Beacon::Beacon(Game *game, EntityRef ref, int ownerId, vector2f pos, State state)
    : Building(game, ref, ownerId, BEACON_COST, BEACON_HEALTH, pos),
      state(state)
{}
Beacon::Beacon(Game *game, EntityRef ref, vchIter *iter) : Building(game, ref, iter)
{
    unpackAndMoveIter(iter);
}

void Beacon::go()
{
    switch (state)
    {
        case Spawning:
        {
            build(BEACON_BUILD_RATE, &game->players[ownerId].credit);

            if (isActive())
            {
                boost::shared_ptr<Gateway> transformed(new Gateway(game, this->ref, this->ownerId, this->pos));
                transformed->completeBuildingInstantly(&this->goldInvested);
                game->killAndReplaceEntity(this->ref, transformed);
            }
        }
        break;
        case Despawning:
        {
            unbuild(BEACON_BUILD_RATE, &game->players[ownerId].credit);

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
// ------- GATEWAY -------
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
        vector2f newUnitPos = this->pos + randomVectorWithMagnitudeRange(20, GATEWAY_RANGE);
        boost::shared_ptr<Unit> littleBabyUnitAwwwwSoCute;
        switch (unitTypechar)
        {
            case PRIME_TYPECHAR:
                littleBabyUnitAwwwwSoCute = boost::shared_ptr<Prime>(new Prime(this->game, this->game->getNextEntityRef(), this->ownerId, newUnitPos));
                break;
            case FIGHTER_TYPECHAR:
                littleBabyUnitAwwwwSoCute = boost::shared_ptr<Fighter>(new Fighter(this->game, this->game->getNextEntityRef(), this->ownerId, newUnitPos));
                break;
            default:
                cout << "Gateway doesn't know how to build that unit..." << endl;
                break;
        }
        if (littleBabyUnitAwwwwSoCute)
        {
            state = DepositTo;
            this->game->entities.push_back(littleBabyUnitAwwwwSoCute);
            this->maybeTargetEntity = littleBabyUnitAwwwwSoCute->ref;
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
            if ((*point - this->pos).getMagnitudeSquared() > pow(GATEWAY_RANGE, 2))
            {
                return;
            }
            boost::shared_ptr<GoldPile> goldpile(new GoldPile(game, game->getNextEntityRef(), *point));
            game->entities.push_back(goldpile);
            target = Target(goldpile);
        }
        else if (auto entityRef = target.castToEntityRef())
        {
            state = DepositTo;
            maybeTargetEntity = *entityRef;
        }
        else
        {
            cout << "Gateway can't cast that Target to a point or entity during cmdDepositTo" << endl;
        }
    }
}
void Gateway::cmdScuttle(EntityRef targetRef)
{
    if (this->isActive())
    {
        if (targetRef == this->ref)
        {
            // replace self with a despawning Beacon
            boost::shared_ptr<Unit> beacon(new Beacon(game, this->ref, this->ownerId, this->pos, Beacon::Despawning));
            beacon->completeBuildingInstantly(&this->goldInvested);
            game->killAndReplaceEntity(this->ref, beacon);
        }
        else
        {
            if (auto entity = entityRefToPtrOrNull(*game, targetRef))
            {
                if (auto unit = boost::dynamic_pointer_cast<Unit, Entity>(entity))
                {
                    if (getAllianceType(this->ownerId, unit) == Owned)
                    {
                        if ((this->pos - unit->pos).getMagnitudeSquared() > pow(GATEWAY_RANGE, 2))
                        {
                            if (auto mobileUnit = boost::dynamic_pointer_cast<MobileUnit, Unit>(unit))
                            {
                                mobileUnit->setTarget(this->ref, GATEWAY_RANGE);
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
                    if ((this->pos - goldpile->pos).getMagnitudeSquared() > pow(GATEWAY_RANGE, 2))
                    {
                        // too far away!
                        state = Idle;
                        maybeTargetEntity = NULL_ENTITYREF;
                    }
                    else
                    {
                        state = Scuttle;
                        maybeTargetEntity = goldpile->ref;
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

    packEntityRef(dest, maybeTargetEntity);
}
void Gateway::unpackAndMoveIter(vchIter *iter)
{
    unsigned char enumInt;
    *iter = unpackFromIter(*iter, "C", &enumInt);
    state = static_cast<State>(enumInt);

    *iter = unpackEntityRef(*iter, &maybeTargetEntity);
}

Gateway::Gateway(Game *game, EntityRef ref, int ownerId, vector2f pos)
    : Building(game, ref, ownerId, GATEWAY_COST, GATEWAY_HEALTH, pos),
      state(Idle), inGameTransferState(NoGoldTransfer),
      maybeTargetEntity(NULL_ENTITYREF)
{}
Gateway::Gateway(Game *game, EntityRef ref, vchIter *iter) : Building(game, ref, iter)
{
    unpackAndMoveIter(iter);
}

void Gateway::go()
{
    inGameTransferState = NoGoldTransfer; // will possibly be updated in the following switch
    switch (state)
    {
        case Idle:
        {
            // search for units near enough to complete
            float rangeSquared = pow(GATEWAY_RANGE, 2);
            for (unsigned int i=0; i<game->entities.size(); i++)
            {
                if (auto unit = boost::dynamic_pointer_cast<Unit, Entity>(game->entities[i]))
                {
                    if (unit->ownerId == this->ownerId)
                        if (unit->getBuiltRatio() < 1)
                            if ((unit->pos - this->pos).getMagnitudeSquared() <= rangeSquared)
                                {
                                    state = DepositTo;
                                    maybeTargetEntity = unit->ref;
                                }
                }
            }
        }
        break;
        case DepositTo:
        {
            if (boost::shared_ptr<Entity> depositingToEntityPtr = entityRefToPtrOrNull(*game, maybeTargetEntity))
            {
                // stop if it's out of range
                if ((depositingToEntityPtr->pos - this->pos).getMagnitudeSquared() > pow(GATEWAY_RANGE, 2))
                {
                    state = Idle;
                    maybeTargetEntity = NULL_ENTITYREF;
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
                            maybeTargetEntity = NULL_ENTITYREF;
                        }
                        if (amountDeposited == 0)
                        {
                            state = Idle;
                            maybeTargetEntity = NULL_ENTITYREF;
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
            if (auto entity = (entityRefToPtrOrNull(*game, maybeTargetEntity)))
            {
                if ((this->pos - entity->pos).getMagnitudeSquared() > pow(GATEWAY_RANGE + DISTANCE_TOL, 2))
                {
                    if (auto mobileUnit = boost::dynamic_pointer_cast<MobileUnit, Entity>(entity))
                    {
                        if (mobileUnit->getTarget().castToEntityRef() == this->ref)
                        {
                            // unit is on its way; do nothing and wait
                        }
                        else
                        {
                            // unit is no longer on its way; revert to Idle state
                            state = Idle;
                            maybeTargetEntity = NULL_ENTITYREF;
                        }
                    }
                    else
                    {
                        cout << "somehow a non-mobile unit ended up out of range for a scuttle cmd..." << endl;
                        state = Idle;
                        maybeTargetEntity = NULL_ENTITYREF;
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
                        maybeTargetEntity = NULL_ENTITYREF;
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

Prime::Prime(Game *game, EntityRef ref, int ownerId, vector2f pos)
    : MobileUnit(game, ref, ownerId, PRIME_COST, PRIME_HEALTH, pos),
      heldGold(PRIME_MAX_GOLD_HELD),
      state(Idle)
{}
Prime::Prime(Game *game, EntityRef ref, vchIter *iter) : MobileUnit(game, ref, iter),
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
    goldTransferState = NoGoldTransfer;
    switch (state)
    {
    case Idle:
        break;
    case PickupGold:
        if (boost::shared_ptr<Entity> e = getTarget().castToEntityPtr(*game))
        {
            if ((e->pos - pos).getMagnitude() <= PRIME_RANGE + DISTANCE_TOL)
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
        break;
    case PutdownGold:
        if (optional<vector2f> point = getTarget().getPointUnlessTargetDeleted(*game))
        {
            if ((*point - pos).getMagnitude() <= PRIME_RANGE + DISTANCE_TOL)
            {
                optional<Coins*> coinsToPushTo;
                bool stopOnTransferZero = false;
                if (auto entity = getTarget().castToEntityPtr(*game))
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
                    boost::shared_ptr<GoldPile> gp(new GoldPile(game, game->getNextEntityRef(), *point));
                    game->entities.push_back(gp);
                    coinsToPushTo = &gp->gold;
                    setTarget(Target(gp->ref), PRIME_RANGE);
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
        if (optional<vector2f> point = getTarget().castToPoint())
        {
            if ((*point - pos).getMagnitude() <= PRIME_RANGE + DISTANCE_TOL)
            {
                // create unit if typechar checks out and change target to new unit
                boost::shared_ptr<Building> buildingToBuild;
                switch (gonnabuildTypechar)
                {
                    case GATEWAY_TYPECHAR:
                        buildingToBuild = boost::shared_ptr<Building>(new Gateway(game, game->getNextEntityRef(), this->ownerId, *point));
                        break;
                }

                if (buildingToBuild)
                {
                    game->entities.push_back(buildingToBuild);
                    setTarget(Target(buildingToBuild), PRIME_RANGE);
                }
                else
                {
                    cout << "Prime refuses to build for that typechar!" << endl;
                    state = Idle;
                }
            }
        }
        else if (boost::shared_ptr<Entity> entity = getTarget().castToEntityPtr(*game))
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

Fighter::Fighter(Game *game, EntityRef ref, int ownerId, vector2f pos)
    : MobileUnit(game, ref, ownerId, FIGHTER_COST, FIGHTER_HEALTH, pos),
      state(Idle), shootCooldown(0), animateShot(None), lastShot(None)
{}
Fighter::Fighter(Game *game, EntityRef ref, vchIter *iter)
    : MobileUnit(game, ref, iter)
{
    unpackAndMoveIter(iter);
}

void Fighter::cmdAttack(EntityRef ref)
{
    state = AttackingUnit;
    setTarget(Target(ref), FIGHTER_RANGE);
}
void Fighter::go()
{
    animateShot = None;
    if (shootCooldown > 0)
        shootCooldown --;

    if (state == AttackingUnit)
    {
        bool returnToIdle = false;
        if (auto targetEntity = getTarget().castToEntityPtr(*this->game)) // will return false if unit died (pointer will be empty)
        {
            if (auto targetUnit = boost::dynamic_pointer_cast<Unit, Entity>(targetEntity))
            {
                vector2f toTarget = (targetUnit->pos - pos);
                angle_view = toTarget.getAngle();
                if (toTarget.getMagnitude() <= FIGHTER_RANGE + DISTANCE_TOL)
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
            state = Idle;
        }
    }
    mobileUnitGo();
}
void Fighter::onMoveCmd(vector2f moveTo)
{
    state = Idle;
}

void Fighter::shootAt(boost::shared_ptr<Unit> unit)
{
    shootCooldown = FIGHTER_SHOOT_COOLDOWN;
    unit->takeHit(FIGHTER_DAMAGE);
}

float Fighter::getSpeed() { return FIGHTER_SPEED; }
float Fighter::getRange() { return FIGHTER_RANGE; }
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



boost::shared_ptr<Entity> unpackFullEntityAndMoveIter(vchIter *iter, unsigned char typechar, Game *game, EntityRef ref)
{
    switch (typechar)
    {
    case NULL_TYPECHAR:
        return boost::shared_ptr<Entity>();
        break;
    case GOLDPILE_TYPECHAR:
        return boost::shared_ptr<Entity>(new GoldPile(game, ref, iter));
        break;
    case BEACON_TYPECHAR:
        return boost::shared_ptr<Entity>(new Beacon(game, ref, iter));
        break;
    case GATEWAY_TYPECHAR:
        return boost::shared_ptr<Entity>(new Gateway(game, ref, iter));
        break;
    case PRIME_TYPECHAR:
        return boost::shared_ptr<Entity>(new Prime(game, ref, iter));
        break;
    case FIGHTER_TYPECHAR:
        return boost::shared_ptr<Entity>(new Fighter(game, ref, iter));
        break;
    }
    throw runtime_error("Trying to unpack an unrecognized entity");
}

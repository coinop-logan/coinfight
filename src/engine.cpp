#include <iostream>
#include <boost/shared_ptr.hpp>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <vector>
#include <string>
#include "myvectors.h"
#include "config.h"
#include "vchpack.h"
#include "engine.h"

//using namespace std;

boost::shared_ptr<Entity> unpackFullEntityAndMoveIter(vchIter *iter, unsigned char typechar, Game *game, EntityRef ref)
{
    switch (typechar)
    {
    case NULL_TYPECHAR:
        return boost::shared_ptr<Entity>();
        break;
    case GATEWAY_TYPECHAR:
        return boost::shared_ptr<Entity>(new Gateway(game, ref, iter));
        break;
    case PRIME_TYPECHAR:
        return boost::shared_ptr<Entity>(new Prime(game, ref, iter));
        break;
    case GOLDPILE_TYPECHAR:
        return boost::shared_ptr<Entity>(new GoldPile(game, ref, iter));
        break;
    default:
        throw runtime_error("Trying to unpack an unrecognized entity");
    }
}

vector<EntityRef> entityPointersToRefs(vector<boost::shared_ptr<Entity>> ptrs)
{
    vector<EntityRef> refs;
    for (uint i = 0; i < ptrs.size(); i++)
    {
        refs.push_back(ptrs[i]->ref);
    }
    return refs;
}

unsigned char getMaybeNullEntityTypechar(boost::shared_ptr<Entity> e)
{
    if (e)
        return e->typechar();
    else
        return NULL_TYPECHAR;
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
Entity::Entity(Game *game, EntityRef ref, vector2f pos) : game(game), ref(ref), pos(pos)
{
    dead = false;
}
Entity::Entity(Game *game, EntityRef ref, vchIter *iter) : game(game), ref(ref)
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

void GoldPile::pack(vch *dest)
{
    packEntity(dest);
    packToVch(dest, "L", amount);
}
void GoldPile::unpackAndMoveIter(vchIter *iter)
{
    *iter = unpackFromIter(*iter, "L", &amount);
}

GoldPile::GoldPile(Game *game, EntityRef ref, vector2f pos, uint32_t amount) : Entity(game, ref, pos),
                                                                               amount(amount) {}
GoldPile::GoldPile(Game *game, EntityRef ref, vchIter *iter) : Entity(game, ref, iter)
{
    unpackAndMoveIter(iter);
}

unsigned int GoldPile::tryDeductAmount(unsigned int attemptedAmount)
{
    if (attemptedAmount < amount)
    {
        amount -= attemptedAmount;
        return attemptedAmount;
    }
    else
    {
        unsigned int left = amount;
        amount = 0;
        return left;
    }
}

unsigned int GoldPile::tryAddAmount(unsigned int attemptedAmount)
{
    uint maybeOverflowed = amount + attemptedAmount;
    uint added;
    if (maybeOverflowed < amount) // indicates overflow
    {
        added = UINT_MAX - amount;
        amount = UINT_MAX;
    }
    else
    {
        added = attemptedAmount;
        amount = maybeOverflowed;
    }
    return added;
}

unsigned char GoldPile::typechar() { return GOLDPILE_TYPECHAR; }
string GoldPile::getTypeName() { return "GoldPile"; }
void GoldPile::go() {}

float Unit::getCreditCost()
{
    throw runtime_error("getCreditCost() has not been defined for " + getTypeName() + ".");
}
void Unit::packUnit(vch *destVch)
{
    packEntity(destVch);
    packToVch(destVch, "f", builtAmount);
}

void Unit::unpackUnitAndMoveIter(vchIter *iter)
{
    *iter = unpackFromIter(*iter, "f", &builtAmount);
}

Unit::Unit(Game *game, EntityRef ref, vector2f pos) : Entity(game, ref, pos)
{
    builtAmount = 0;
}
Unit::Unit(Game *game, EntityRef ref, vchIter *iter) : Entity(game, ref, iter)
{
    unpackUnitAndMoveIter(iter);
}

float Unit::build(float attemptedAmount)
{
    if (builtAmount + attemptedAmount <= getCreditCost())
    {
        builtAmount += attemptedAmount;
        return attemptedAmount;
    }
    else
    {
        float amount = getCreditCost() - builtAmount;
        builtAmount = getCreditCost();
        return amount;
    }
}

float Unit::getBuiltAmount()
{
    return builtAmount;
}
bool Unit::isActive()
{
    return (builtAmount >= getCreditCost());
}

void Building::packBuilding(vch *destVch)
{
    packUnit(destVch);
}
void Building::unpackBuildingAndMoveIter(vchIter *iter)
{
}

Building::Building(Game *game, uint16_t ref, vector2f pos) : Unit(game, ref, pos) {}
Building::Building(Game *game, uint16_t ref, vchIter *iter) : Unit(game, ref, iter)
{
    unpackBuildingAndMoveIter(iter);
}

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

MobileUnit::MobileUnit(Game *game, uint16_t ref, vector2f pos) : Unit(game, ref, pos),
                                                                 target(NULL_ENTITYREF)
{
    targetRange = 0;
}
MobileUnit::MobileUnit(Game *game, uint16_t ref, vchIter *iter) : Unit(game, ref, iter),
                                                                  target(NULL_ENTITYREF)
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
    if (optional<vector2f> p = target.getPoint(game))
        moveTowardPoint(*p, targetRange);
    else
        setTarget(Target(pos), 0);
}
void MobileUnit::cmdMove(vector2f pointTarget)
{
    setTarget(Target(pointTarget), 0);
}

void Prime::pack(vch *dest)
{
    packMobileUnit(dest);
    packToVch(dest, "C", (unsigned char)(state));
    packToVch(dest, "L", heldCredit);
}
void Prime::unpackAndMoveIter(vchIter *iter)
{
    unsigned char enumInt;
    *iter = unpackFromIter(*iter, "C", &enumInt);
    state = static_cast<State>(enumInt);

    *iter = unpackFromIter(*iter, "L", &heldCredit);
}

Prime::Prime(Game *game, uint16_t ref, vector2f pos) : MobileUnit(game, ref, pos)
{
    state = Idle;
    heldCredit = 0;
}
Prime::Prime(Game *game, uint16_t ref, vchIter *iter) : MobileUnit(game, ref, iter)
{
    unpackAndMoveIter(iter);
}

void Prime::cmdPickup(EntityRef goldRef)
{
    state = PickupGold;

    setTarget(Target(goldRef), PRIME_RANGE);
}

void Prime::cmdPutdown(Target _target)
{
    state = PutdownGold;

    setTarget(_target, PRIME_RANGE);
}
void Prime::cmdPutdownForGateway(boost::shared_ptr<Gateway> gateway)
{
    // create empty gold pile at Gateway's range, toward self
    cout << "gateway: " << gateway->getPos().x << "," << gateway->getPos().y << endl;
    vector2f gatewayToPrime = this->getPos() - gateway->getPos();
    cout << "gatewayToPrime: " << gatewayToPrime.x << "," << gatewayToPrime.y << endl;
    vector2f resized = gatewayToPrime.normalized() * GATEWAY_RANGE;
    cout << "resized: " << resized.x << "," << resized.y << endl;
    vector2f pilePos = gateway->getPos() + resized;
    cout << "pilePos: " << pilePos.x << "," << pilePos.y << endl;

    EntityRef goldRef = game->getNextEntityRef();
    boost::shared_ptr<GoldPile> goldPile(new GoldPile(game, goldRef, pilePos, 0));
    game->entities.push_back(goldPile);

    // tell gateway to target that pile
    gateway->reclaimGoldPile(goldPile);

    state = PutdownGold;
    setTarget(Target(goldRef), PRIME_RANGE);
}

unsigned int Prime::tryDeductAmount(unsigned int attemptedAmount)
{
    if (attemptedAmount < heldCredit)
    {
        heldCredit -= attemptedAmount;
        return attemptedAmount;
    }
    else
    {
        unsigned int left = heldCredit;
        heldCredit = 0;
        return left;
    }
}

float Prime::getSpeed() { return PRIME_SPEED; }
float Prime::getRange() { return PRIME_RANGE; }

unsigned char Prime::typechar() { return PRIME_TYPECHAR; }
string Prime::getTypeName() { return "Prime"; }
float Prime::getCreditCost() { return PRIME_COST; }

void Prime::go()
{
    switch (state)
    {
    case Idle:
        break;
    case PickupGold:
        if (boost::shared_ptr<Entity> e = getTarget().castToEntityPtr(game))
        {
            if ((e->pos - pos).getMagnitude() <= PRIME_RANGE + DISTANCE_TOL)
            {
                if (boost::shared_ptr<GoldPile> gp = boost::dynamic_pointer_cast<GoldPile, Entity>(e))
                {
                    heldCredit += gp->tryDeductAmount(PRIME_PICKUP_RATE);
                }
            }
        }
        break;
    case PutdownGold:
        if (optional<vector2f> point = getTarget().getPoint(game))
        {
            if (boost::shared_ptr<Entity> e = getTarget().castToEntityPtr(game))
            {
                if ((*point - pos).getMagnitude() <= PRIME_RANGE + DISTANCE_TOL)
                {
                    if (boost::shared_ptr<GoldPile> gp = boost::dynamic_pointer_cast<GoldPile, Entity>(e))
                    {
                        // goldPile already exists
                        unsigned int amountToAdd = tryDeductAmount(PRIME_PUTDOWN_RATE);
                        if (amountToAdd > 0)
                        {
                            gp->tryAddAmount(amountToAdd);
                        }
                        else
                        {
                            state = Idle;
                        }
                    }
                    else
                    {
                        // trying to put down gold "on" non-gold entity...
                        // maybe in future can transfer to some other unit?
                    }
                }
            }
            else
            {
                // must create goldPile
                boost::shared_ptr<GoldPile> gp(new GoldPile(game, game->getNextEntityRef(), *point, 0));
                game->entities.push_back(gp);
                setTarget(Target(gp->ref), PRIME_RANGE);
            }
        }
        break;
    default:
        throw logic_error("This case not handled in Prime::go()");
    }
    mobileUnitGo();
}

void Gateway::iterateSpawning()
{
    //check if void
    if (entityRefIsNull(targetRef))
    {
        throw logic_error("trying to iterateSpawning, but there is no spawningPrime pointer!");
    }
    //stop if complete
    if (spawningPrime()->isActive())
    {
        state = Idle;
        spawningPrime().reset();
    }
    else
    {
        // iterate
        float amountToInvest = min(game->playerCredit, GATEWAY_TRANSFER_RATE);
        float built = spawningPrime()->build(amountToInvest);

        game->playerCredit -= built;
    }
}

unsigned char Gateway::typechar() { return GATEWAY_TYPECHAR; }
string Gateway::getTypeName() { return "Gateway"; }
float Gateway::getCreditCost() { return GATEWAY_COST; }

void Gateway::pack(vch *dest)
{
    packBuilding(dest);
    packToVch(dest, "C", (unsigned char)(state));
    packEntityRef(dest, targetRef);
}
void Gateway::unpackAndMoveIter(vchIter *iter)
{
    unsigned char enumInt;
    *iter = unpackFromIter(*iter, "C", &enumInt);
    state = static_cast<State>(enumInt);

    *iter = unpackEntityRef(*iter, &targetRef);
}

Gateway::Gateway(Game *game, uint16_t ref, vector2f pos, bool alreadyCompleted) : Building(game, ref, pos)
{
    if (alreadyCompleted)
    {
        builtAmount = getCreditCost();
    }
}
Gateway::Gateway(Game *game, uint16_t ref, vchIter *iter) : Building(game, ref, iter)
{
    unpackAndMoveIter(iter);
}

void Gateway::go()
{
    switch (state)
    {
    case Idle:
        break;
    case Spawning:
        iterateSpawning();
        break;
    case Reclaiming:
        if (boost::shared_ptr<GoldPile> goldPile = boost::dynamic_pointer_cast<GoldPile, Entity>(game->entityRefToPtr(targetRef)))
        {
            if ((goldPile->pos - pos).getMagnitude() <= GATEWAY_RANGE + DISTANCE_TOL)
            {
                game->playerCredit += goldPile->tryDeductAmount(GATEWAY_TRANSFER_RATE);

                // This may have depleted and killed the pile
                if (goldPile->dead)
                {
                    state = Idle;
                }
            }
        }
        break;
    default:
        throw runtime_error("You haven't defined what the Gateway should be doing in this state");
    }
}

boost::shared_ptr<Prime> Gateway::spawningPrime()
{
    boost::shared_ptr<Prime> p = boost::dynamic_pointer_cast<Prime, Entity>(game->entityRefToPtr(targetRef));
    return p;
}

void Gateway::startSpawningPrime(vector2f primePos)
{
    state = Spawning;

    targetRef = game->getNextEntityRef();

    game->entities.push_back(boost::shared_ptr<Prime>(new Prime(game, targetRef, primePos)));
}

void Gateway::reclaimGoldPile(boost::shared_ptr<GoldPile> goldPile)
{
    state = Reclaiming;

    targetRef = goldPile->ref;
}

boost::shared_ptr<Entity> Game::entityRefToPtr(EntityRef r)
{
    if (r == 0)
    {
        return boost::shared_ptr<Entity>();
    }
    else
    {
        return entities[r - 1];
    }
}
EntityRef Game::getNextEntityRef()
{
    return entities.size() + 1;
}

void Game::pack(vch *dest)
{
    packToVch(dest, "Q", frame);

    packToVch(dest, "f", playerCredit);
    packToVch(dest, "H", (EntityRef)(entities.size()));

    for (EntityRef i = 0; i < entities.size(); i++)
    {
        unsigned char typechar = getMaybeNullEntityTypechar(entities[i]);

        packTypechar(dest, typechar);

        if (typechar != NULL_TYPECHAR)
        {
            entities[i]->pack(dest);
        }
    }
}
void Game::unpackAndMoveIter(vchIter *iter)
{
    *iter = unpackFromIter(*iter, "Q", &frame);
    *iter = unpackFromIter(*iter, "f", &playerCredit);

    uint16_t entitiesSize;
    *iter = unpackFromIter(*iter, "H", &entitiesSize);
    cout << "s:" << entitiesSize << endl;
    entities.clear();

    for (int i = 0; i < entitiesSize; i++)
    {
        cout << i << "|" << endl;
        unsigned char typechar;
        *iter = unpackTypecharFromIter(*iter, &typechar);

        entities.push_back(unpackFullEntityAndMoveIter(iter, typechar, this, getNextEntityRef()));
    }
}

Game::Game()
{
}
Game::Game(vchIter *iter)
{
    unpackAndMoveIter(iter);
}

void Game::reassignEntityGamePointers()
{
    for (EntityRef i = 0; i < entities.size(); i++)
    {
        if (entities[i])
            entities[i]->game = this;
    }
}

void Game::testInit()
{
    frame = 0;
    playerCredit = 50;

    boost::shared_ptr<Gateway> g(new Gateway(this, 1, vector2f(10, 12), true));
    entities.push_back(g);
    boost::shared_ptr<GoldPile> gp(new GoldPile(this, 2, vector2f(200, 50), 1000));
    entities.push_back(gp);

    g->startSpawningPrime(vector2f(20, 20));
}

void Game::iterate()
{
    for (uint i = 0; i < entities.size(); i++)
    {
        if (entities[i])
            entities[i]->go();
    }

    for (uint i = 0; i < entities.size(); i++)
    {
        if (entities[i] && entities[i]->dead)
            entities[i].reset();
    }

    frame++;

    cout << "c:" << playerCredit << endl;
}

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

optional<vector2f> Target::getPoint(Game *game)
{
    if (type == PointTarget)
    {
        return {pointTarget};
    }
    else if (boost::shared_ptr<Entity> e = game->entityRefToPtr(entityTarget))
    {
        return {e->pos};
    }
    else
    {
        return {};
    }
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

boost::shared_ptr<Entity> Target::castToEntityPtr(Game *game)
{
    if (auto eRef = castToEntityRef())
        return game->entityRefToPtr(*eRef);
    else
        return boost::shared_ptr<Entity>();
}
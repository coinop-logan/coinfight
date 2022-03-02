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
#include "coins.h"

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

vector<EntityRef> entityPtrsToRefs(vector<boost::shared_ptr<Entity>> ptrs)
{
    vector<EntityRef> refs;
    for (uint i = 0; i < ptrs.size(); i++)
    {
        refs.push_back(ptrs[i]->ref);
    }
    return refs;
}

boost::shared_ptr<Entity> entityRefToPtr(const Game& game, EntityRef ref)
{
    if (ref == 0)
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

void GoldPile::pack(vch *dest)
{
    packEntity(dest);
    gold.pack(dest);
}
void GoldPile::unpackAndMoveIter(vchIter *iter)
{
    gold = Coins(iter);
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
void GoldPile::go() {}

coinsInt Unit::getCost()
{
    throw runtime_error("getCost() has not been defined for " + getTypeName() + ".");
}
void Unit::packUnit(vch *destVch)
{
    packEntity(destVch);
    goldInvested.pack(destVch);
}

void Unit::unpackUnitAndMoveIter(vchIter *iter)
{
    goldInvested = Coins(iter);
}

Unit::Unit(Game *game, EntityRef ref, coinsInt totalCost, vector2f pos) : Entity(game, ref, pos), goldInvested(totalCost) {}
Unit::Unit(Game *game, EntityRef ref, vchIter *iter) : Entity(game, ref, iter),
                                                       goldInvested((coinsInt)0) // will get overwritten in unpack below
{
    unpackUnitAndMoveIter(iter);
}

coinsInt Unit::build(coinsInt attemptedAmount, Coins *fromCoins)
{
    return fromCoins->transferUpTo(attemptedAmount, &(this->goldInvested));
}
bool Unit::completeBuildingInstantly(Coins* fromCoins)
{
    return fromCoins->tryTransfer(this->getCost() - this->goldInvested.getInt(), &this->goldInvested);
}

coinsInt Unit::getBuilt()
{
    return goldInvested.getInt();
}
bool Unit::isActive()
{
    return
    (
        getBuilt() >= getCost() &&
        (!dead)
    );
}

void Building::packBuilding(vch *destVch)
{
    packUnit(destVch);
}
void Building::unpackBuildingAndMoveIter(vchIter *iter)
{
}

Building::Building(Game *game, uint16_t ref, coinsInt totalCost, vector2f pos) : Unit(game, ref, totalCost, pos) {}
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

MobileUnit::MobileUnit(Game *game, uint16_t ref, coinsInt totalCost, vector2f pos) : Unit(game, ref, totalCost, pos),
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
    if (optional<vector2f> p = target.getPoint(*game))
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

    heldGold.pack(dest);
}
void Prime::unpackAndMoveIter(vchIter *iter)
{
    unsigned char enumInt;
    *iter = unpackFromIter(*iter, "C", &enumInt);
    state = static_cast<State>(enumInt);

    heldGold = Coins(iter);
}

Prime::Prime(Game *game, uint16_t ref, vector2f pos) : MobileUnit(game, ref, PRIME_COST, pos),
                                                       heldGold(PRIME_MAX_GOLD_HELD),
                                                       drawLineToTarget(false),
                                                       state(Idle)
{}
Prime::Prime(Game *game, uint16_t ref, vchIter *iter) : MobileUnit(game, ref, iter),
                                                        heldGold(PRIME_MAX_GOLD_HELD)
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
void Prime::cmdSendGoldThroughGateway(boost::shared_ptr<Gateway> gateway)
{
    state = SendGoldThroughGateway;
    setTarget(Target(gateway->ref), PRIME_RANGE);
}
void Prime::cmdPushGoldThroughGateway(boost::shared_ptr<Gateway> gateway)
{
    state = PushGoldThroughGateway;
    setTarget(Target(gateway->ref), PRIME_RANGE);
}
void Prime::cmdBuild(boost::shared_ptr<Building> building)
{
    state = BuildingBuilding;
    setTarget(Target(building->ref), PRIME_RANGE);
}

float Prime::getSpeed() { return PRIME_SPEED; }
float Prime::getRange() { return PRIME_RANGE; }
coinsInt Prime::getCost() { return PRIME_COST; }

unsigned char Prime::typechar() { return PRIME_TYPECHAR; }
string Prime::getTypeName() { return "Prime"; }

void Prime::go()
{
    drawLineToTarget = false;
    switch (state)
    {
    case Idle:
        break;
    case PickupGold:
        if (boost::shared_ptr<Entity> e = getTarget().castToEntityPtr(*game))
        {
            if ((e->pos - pos).getMagnitude() <= PRIME_RANGE + DISTANCE_TOL)
            {
                if (boost::shared_ptr<GoldPile> gp = boost::dynamic_pointer_cast<GoldPile, Entity>(e))
                {
                    int amount = gp->gold.transferUpTo(PRIME_PICKUP_RATE, &(this->heldGold));
                    cout << "Prime picking up " << amount << endl;
                    if (amount > 0)
                    {
                        drawLineToTarget = true;
                    }
                }
            }
        }
        break;
    case PutdownGold:
        if (optional<vector2f> point = getTarget().getPoint(*game))
        {
            if (boost::shared_ptr<Entity> e = getTarget().castToEntityPtr(*game))
            {
                if ((*point - pos).getMagnitude() <= PRIME_RANGE + DISTANCE_TOL)
                {
                    if (boost::shared_ptr<GoldPile> gp = boost::dynamic_pointer_cast<GoldPile, Entity>(e))
                    {
                        // goldPile already exists
                        coinsInt amountPutDown = this->heldGold.transferUpTo(PRIME_PUTDOWN_RATE, &(gp->gold));
                        cout << "Prime putting down " << amountPutDown << endl;
                        if (amountPutDown == 0)
                        {
                            state = Idle;
                        }
                        else
                        {
                            drawLineToTarget = true;
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
                boost::shared_ptr<GoldPile> gp(new GoldPile(game, game->getNextEntityRef(), *point));
                game->entities.push_back(gp);
                setTarget(Target(gp->ref), PRIME_RANGE);
            }
        }
        break;
    case SendGoldThroughGateway:
        if (boost::shared_ptr<Entity> e = getTarget().castToEntityPtr(*game))
        {
            if ((e->pos - pos).getMagnitude() <= PRIME_RANGE + DISTANCE_TOL)
            {
                if (boost::shared_ptr<Gateway> gw = boost::dynamic_pointer_cast<Gateway, Entity>(e))
                {
                    // cout << "held gold: " << this->heldGold.getInt() << endl;
                    // cout << "player credit: " << game->playerCredit.getInt() << endl;
                    int amount = this->heldGold.transferUpTo(PRIME_PUTDOWN_RATE, &(game->playerCredit));
                    cout << "Sending through gateway: " << amount << endl;
                    if (amount > 0)
                        drawLineToTarget = true;
                }
            }
        }
        break;
    case PushGoldThroughGateway:
        if (boost::shared_ptr<Entity> e = getTarget().castToEntityPtr(*game))
        {
            if ((e->pos - pos).getMagnitude() <= PRIME_RANGE + DISTANCE_TOL)
            {
                if (boost::shared_ptr<Gateway> gw = boost::dynamic_pointer_cast<Gateway, Entity>(e))
                {
                    // cout << "held gold: " << this->heldGold.getInt() << endl;
                    // cout << "player credit: " << game->playerCredit.getInt() << endl;
                    int amount = game->playerCredit.transferUpTo(PRIME_PUTDOWN_RATE, &(this->heldGold));
                    cout << "Pushing through gateway: " << amount << endl;
                    if (amount > 0)
                        drawLineToTarget = true;
                }
            }
        }
        break;
    case BuildingBuilding:
        if (boost::shared_ptr<Entity> e = getTarget().castToEntityPtr(*game))
        {
            if ((e->pos - pos).getMagnitude() <= PRIME_RANGE + DISTANCE_TOL)
            {
                if (boost::shared_ptr<Building> b = boost::dynamic_pointer_cast<Building, Entity>(e))
                {
                    coinsInt leftToBuild = b->getCost() - b->getBuilt();
                    if (leftToBuild == 0)
                    {
                        state = Idle;
                        cout << "built!" << endl;
                    }
                    else
                    {
                        coinsInt amountBuilt = b->build(PRIME_PICKUP_RATE, &(this->heldGold));
                        cout << "building " << amountBuilt << endl;
                        if (amountBuilt == 0)
                        {
                            state = Idle;
                        }
                    }
                }
            }
        }
        break;
    default:
        throw logic_error("This case not handled in Prime::go()");
    }
    mobileUnitGo();
}

unsigned char Gateway::typechar() { return GATEWAY_TYPECHAR; }
string Gateway::getTypeName() { return "Gateway"; }
coinsInt Gateway::getCost() { return GATEWAY_COST; }

void Gateway::pack(vch *dest)
{
    packBuilding(dest);
}
void Gateway::unpackAndMoveIter(vchIter *iter)
{}

Gateway::Gateway(Game *game, uint16_t ref, vector2f pos) : Building(game, ref, GATEWAY_COST, pos)
{}
Gateway::Gateway(Game *game, uint16_t ref, vchIter *iter) : Building(game, ref, iter)
{
    unpackAndMoveIter(iter);
}

void Gateway::go()
{}


EntityRef Game::getNextEntityRef()
{
    return entities.size() + 1;
}

void Game::pack(vch *dest)
{
    packToVch(dest, "Q", frame);

    playerCredit.pack(dest);

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
    
    playerCredit = Coins(iter);

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

Game::Game() : playerCredit(MAX_COINS) {}
Game::Game(vchIter *iter) : playerCredit(MAX_COINS)
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
    playerCredit.createMoreByFiat(35500);

    boost::shared_ptr<Gateway> g(new Gateway(this, 1, vector2f(20, 20)));
    boost::shared_ptr<Prime> p1(new Prime(this, 2, vector2f(50, 50)));
    boost::shared_ptr<Prime> p2(new Prime(this, 3, vector2f(70, 50)));

    boost::shared_ptr<GoldPile> gp1(new GoldPile(this, 4, vector2f(150, 50)));
    boost::shared_ptr<GoldPile> gp2(new GoldPile(this, 5, vector2f(200, 60)));
    boost::shared_ptr<GoldPile> gp3(new GoldPile(this, 6, vector2f(200, 200)));

    if (!
        (
          g->completeBuildingInstantly(&playerCredit)
       && playerCredit.tryTransfer(500, &gp1->gold)
       && playerCredit.tryTransfer(1000, &gp2->gold)
       && playerCredit.tryTransfer(4000, &gp3->gold)
       && p1->completeBuildingInstantly(&playerCredit)
       && p2->completeBuildingInstantly(&playerCredit)
        )
    )
        throw runtime_error("not enough playerCredit to initialize all of that");

    // ordering is important due to ref IDs passed earlier!
    entities.push_back(g);
    entities.push_back(p1);
    entities.push_back(p2);
    entities.push_back(gp1);
    entities.push_back(gp2);
    entities.push_back(gp3);
}

void Game::iterate()
{
    for (uint i = 0; i < entities.size(); i++)
    {
        if (entities[i])
        {
            entities[i]->go();
        }
    }

    for (uint i = 0; i < entities.size(); i++)
    {
        if (entities[i] && entities[i]->dead)
        {
            if (boost::shared_ptr<Unit> unit = boost::dynamic_pointer_cast<Unit, Entity>(entities[i]))
            {
                boost::shared_ptr<GoldPile> gp(new GoldPile(this, getNextEntityRef(), unit->getPos()));
                if (!unit->goldInvested.tryTransfer(unit->goldInvested.getInt(), &gp->gold))
                {
                    throw runtime_error("Error when trying to transfer a dead unit's gold into a new goldPile");
                }
                entities.push_back(gp);
            }
            entities[i].reset();
        }
    }

    frame++;

    if (frame % 40 == 0)
    {
        cout << "c:" << playerCredit.getInt() << endl;
    }
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

optional<vector2f> Target::getPoint(const Game &game)
{
    if (type == PointTarget)
    {
        return {pointTarget};
    }
    else if (boost::shared_ptr<Entity> e = entityRefToPtr(game, entityTarget))
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

boost::shared_ptr<Entity> Target::castToEntityPtr(const Game &game)
{
    if (auto eRef = castToEntityRef())
        return entityRefToPtr(game, *eRef);
    else
        return boost::shared_ptr<Entity>();
}
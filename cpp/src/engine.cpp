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
#include "events.h"

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
    case FIGHTER_TYPECHAR:
        return boost::shared_ptr<Entity>(new Fighter(game, ref, iter));
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

boost::shared_ptr<Entity> entityRefToPtrOrNull(const Game& game, EntityRef ref)
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
            return Ally;
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
sf::Color Entity::getPrimaryColor()
{
    throw runtime_error("getPrimaryColor() has not been defined for " + getTypeName() + ".");
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
Coins* Entity::getDroppableCoins()
{
    throw runtime_error("getDroppableCoins has not been defined for " + getTypeName() + ".");
}

Coins* GoldPile::getDroppableCoins()
{
    return &gold;
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
sf::Color GoldPile::getPrimaryColor()
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

sf::Color playerAddressToColor(string address)
{
    int vals[3];
    for (uint i=0; i<3; i++)
    {
        string charStr = address.substr(2 + i, 1);
        unsigned int intVal = std::stoul(charStr, nullptr, 16);
        vals[i] = 55 + ((intVal / 15.0) * 200);
    }
    return sf::Color(vals[0], vals[1], vals[2]);
}

Coins* Unit::getDroppableCoins()
{
    return &goldInvested;
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
    : Entity(game, ref, pos), ownerId(ownerId), health(health), goldInvested(totalCost) {}

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
sf::Color Unit::getPrimaryColor()
{
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
    cout << "took a hit! Health at " << health << endl;
}
uint16_t Unit::getHealth() { return health; }

void Building::packBuilding(vch *destVch)
{
    packUnit(destVch);
}
void Building::unpackBuildingAndMoveIter(vchIter *iter)
{
}

Building::Building(Game *game, uint16_t ref, int ownerId, coinsInt totalCost, uint16_t health, vector2f pos)
    : Unit(game, ref, ownerId, totalCost, health, pos) {}
Building::Building(Game *game, uint16_t ref, vchIter *iter) : Unit(game, ref, iter)
{
    unpackBuildingAndMoveIter(iter);
}

void Building::buildingGo()
{
    unitGo();
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

MobileUnit::MobileUnit(Game *game, uint16_t ref, int ownerId, coinsInt totalCost, uint16_t health, vector2f pos)
    : Unit(game, ref, ownerId, totalCost, health, pos), target(NULL_ENTITYREF), angle_view(0)
{
    targetRange = 0;
    setTarget(Target(pos), 0);
}
MobileUnit::MobileUnit(Game *game, uint16_t ref, vchIter *iter) : Unit(game, ref, iter),
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

Prime::Prime(Game *game, uint16_t ref, int ownerId, vector2f pos)
    : MobileUnit(game, ref, ownerId, PRIME_COST, PRIME_HEALTH, pos),
      heldGold(PRIME_MAX_GOLD_HELD),
      state(Idle)
{}
Prime::Prime(Game *game, uint16_t ref, vchIter *iter) : MobileUnit(game, ref, iter),
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

float Prime::getSpeed() { return PRIME_SPEED; }
float Prime::getRange() { return PRIME_RANGE; }
coinsInt Prime::getCost() { return PRIME_COST; }
uint16_t Prime::getMaxHealth() { return PRIME_HEALTH; }

unsigned char Prime::typechar() { return PRIME_TYPECHAR; }
string Prime::getTypeName() { return "Prime"; }

void Prime::go()
{
    goldTransferState = None;
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
        if (this->heldGold.getInt() == 0)
        {
            state = Idle;
        }
        else if (optional<vector2f> point = getTarget().getPointUnlessTargetDeleted(*game))
        {
            if ((*point - pos).getMagnitude() <= PRIME_RANGE + DISTANCE_TOL)
            {
                optional<Coins*> coinsToPushTo;
                if (auto entity = getTarget().castToEntityPtr(*game))
                {
                    if (auto goldpile = boost::dynamic_pointer_cast<GoldPile, Entity>(entity))
                    {
                        coinsToPushTo = &goldpile->gold;
                    }
                    else if (auto gateway = boost::dynamic_pointer_cast<Gateway, Entity>(entity))
                    {
                        if (gateway->ownerId == this->ownerId)
                        {
                            coinsToPushTo = &game->players[gateway->ownerId].credit;
                        }
                    }
                    else
                    {
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
                    if (amountPutDown == 0)
                    {
                        state = Idle;
                    }
                    else
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
    default:
        throw logic_error("This case not handled in Prime::go()");
    }
    mobileUnitGo();
}

    enum State
    {
        Idle,
        AttackingUnit
    } state;

    int shootCooldown;

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

void Fighter::cmdAttack(boost::shared_ptr<Unit> targetedUnit)
{
    state = AttackingUnit;
    setTarget(Target(targetedUnit->ref), FIGHTER_RANGE);
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
            cout << "fighter returning to idle" << endl;
            state = Idle;
        }
    }
    mobileUnitGo();
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


unsigned char Gateway::typechar() { return GATEWAY_TYPECHAR; }
string Gateway::getTypeName() { return "Gateway"; }
coinsInt Gateway::getCost() { return GATEWAY_COST; }
uint16_t Gateway::getMaxHealth() { return GATEWAY_HEALTH; }

void Gateway::cmdBuildUnit(unsigned char unitTypechar)
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
    this->game->entities.push_back(littleBabyUnitAwwwwSoCute);
    this->maybeBuildingUnit = littleBabyUnitAwwwwSoCute;
}
float Gateway::buildQueueWeight()
{
    if (!maybeBuildingUnit)
        return 0;
    else
        return 1;
}

void Gateway::pack(vch *dest)
{
    packBuilding(dest);
}
void Gateway::unpackAndMoveIter(vchIter *iter)
{}

Gateway::Gateway(Game *game, uint16_t ref, int ownerId, vector2f pos)
    : Building(game, ref, ownerId, GATEWAY_COST, FIGHTER_COST, pos)
{}
Gateway::Gateway(Game *game, uint16_t ref, vchIter *iter) : Building(game, ref, iter)
{
    unpackAndMoveIter(iter);
}

void Gateway::go()
{
    if (maybeBuildingUnit)
    {
        coinsInt builtAmount = maybeBuildingUnit->build(GATEWAY_BUILD_RATE, &game->players[this->ownerId].credit);
        if (builtAmount == 0)
        {
            maybeBuildingUnit.reset();
        }
    }
}


EntityRef Game::getNextEntityRef()
{
    return entities.size() + 1;
}

void Player::pack(vch *dest)
{
    packStringToVch(dest, address);
    credit.pack(dest);
}
void Player::unpackAndMoveIter(vchIter *iter)
{
    *iter = unpackStringFromIter(*iter, 50, &address);
    credit = Coins(iter);
}

Player::Player(string address)
    : address(address), credit() {}

Player::Player(vchIter *iter)
{
    unpackAndMoveIter(iter);
}

int Game::playerAddressToIdOrNegativeOne(string address)
{
    for (uint i=0; i<players.size(); i++)
    {
        if (players[i].address == address)
        {
            return i;
        }
    }
    return -1;
}
string Game::playerIdToAddress(uint playerId)
{
    return players[playerId].address;
}

void Game::pack(vch *dest)
{
    packToVch(dest, "C", (unsigned char)(state));
    packToVch(dest, "Q", frame);

    packToVch(dest, "C", (unsigned char)(players.size()));
    for (uint i=0; i < players.size(); i++)
    {
        players[i].pack(dest);
    }

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
    unsigned char enumInt;
    *iter = unpackFromIter(*iter, "C", &enumInt);
    state = static_cast<State>(enumInt);

    *iter = unpackFromIter(*iter, "Q", &frame);
    
    uint8_t playersSize;
    *iter = unpackFromIter(*iter, "C", &playersSize);
    players.clear();

    for (int i = 0; i < playersSize; i++)
    {
        players.push_back(Player(iter));
    }

    uint16_t entitiesSize;
    *iter = unpackFromIter(*iter, "H", &entitiesSize);
    entities.clear();

    for (int i = 0; i < entitiesSize; i++)
    {
        unsigned char typechar;
        *iter = unpackTypecharFromIter(*iter, &typechar);

        entities.push_back(unpackFullEntityAndMoveIter(iter, typechar, this, getNextEntityRef()));
    }
}

Game::Game() : state(Pregame) {}
Game::Game(vchIter *iter)
{
    unpackAndMoveIter(iter);
}

void Game::startMatchOrPrintError()
{
    if (players.size() == 0)
    {
        cout << "No players to start the game with!" << endl;
        return;
    }

    coinsInt neededCreditPerPlayer = PRIME_COST + GATEWAY_COST;
    for (uint i=0; i<players.size(); i++)
    {
        if (players[i].credit.getInt() < neededCreditPerPlayer)
        {
            cout << "Player " << players[i].address << " doesn't have enough credit! They currently have " << players[i].credit.getInt() << " but they need " << neededCreditPerPlayer << "." << endl;
            return;            
        }
    }

    float spawnCircleCircumference = SPACE_BETWEEN_SPAWNS * players.size();
    float spawnCircleRadius = spawnCircleCircumference / 2;
    for (uint i=0; i<players.size(); i++)
    {
        float positionAlongCircumference = ((float)i)/players.size();
        float spawnAngle = positionAlongCircumference * 2 * M_PI;
        vector2f spawnPos = composeVector2f(spawnAngle, spawnCircleRadius);

        // if you're going to change this, you should change neededCostPerPlayer above too!
        boost::shared_ptr<Unit> primeUnit(new Prime(this, getNextEntityRef(), i, spawnPos));
        entities.push_back(primeUnit);
        boost::shared_ptr<Unit> gatewayUnit(new Gateway(this, getNextEntityRef(), i, spawnPos + vector2f(50, 50)));
        entities.push_back(gatewayUnit);

        if (!(primeUnit->completeBuildingInstantly(&players[i].credit) && gatewayUnit->completeBuildingInstantly(&players[i].credit)))
        {
            throw logic_error("This player doesn't have enough credit for the starting units - but I thought we checked that in the previous for loop!");
        }
    }

    state = Active;
    cout << "game starting!" << endl;
}

void Game::reassignEntityGamePointers()
{
    for (EntityRef i = 0; i < entities.size(); i++)
    {
        if (entities[i])
            entities[i]->game = this;
    }
}

void Game::iterate()
{
    switch (state)
    {
        case Pregame:
            break;
        case Active:
            for (uint i = 0; i < entities.size(); i++)
            {
                if (entities[i])
                {
                    if (auto unit = boost::dynamic_pointer_cast<Unit, Entity>(entities[i]))
                    {
                        if (unit->isActive())
                            unit->go();
                    }
                    else
                        entities[i]->go();
                }
            }

            for (uint i = 0; i < entities.size(); i++)
            {
                if (entities[i] && entities[i]->dead)
                {
                    Coins *coins = entities[i]->getDroppableCoins();
                    if (coins->getInt() > 0)
                    {
                        boost::shared_ptr<GoldPile> goldPile(new GoldPile(this, getNextEntityRef(), entities[i]->pos));
                        entities.push_back(goldPile);
                        coins->transferUpTo(coins->getInt(), &goldPile->gold);
                    }
                    entities[i].reset();
                }
            }

            frame++;

            if (frame % 200 == 0)
            {
                for (uint i=0; i<players.size(); i++)
                {
                    cout << "player " << players[i].address << " has " << players[i].credit.getInt() << endl;
                }
            }
            break;
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
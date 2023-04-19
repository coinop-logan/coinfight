#include <stdexcept>
#include <boost/shared_ptr.hpp>
#include "cmds.h"
#include "common.h"

using namespace std;

optional<boost::shared_ptr<Cmd>> consumeCmd(Netpack::Consumer* from)
{
    uint8_t typechar = consumeTypechar(from);

    switch (typechar)
    {
    case CMD_SPAWNBEACON_CHAR:
        return boost::shared_ptr<Cmd>(new SpawnBeaconCmd(from));;
    case CMD_WITHDRAW_CHAR:
        return boost::shared_ptr<Cmd>(new WithdrawCmd(from));
    case CMD_MOVE_CHAR:
        return boost::shared_ptr<Cmd>(new MoveCmd(from));
    case CMD_ATTACKSCUTTLE_CHAR:
        return boost::shared_ptr<Cmd>(new AttackScuttleCmd(from));
    case CMD_STOP_CHAR:
        return boost::shared_ptr<Cmd>(new StopCmd(from));
    case CMD_DEPOSIT_CHAR:
        return boost::shared_ptr<Cmd>(new DepositCmd(from));
    case CMD_FETCH_CHAR:
        return boost::shared_ptr<Cmd>(new FetchCmd(from));
    case CMD_GATEWAYBUILD_CHAR:
        return boost::shared_ptr<Cmd>(new GatewayBuildCmd(from));
    case CMD_PRIMEBUILD_CHAR:
        return boost::shared_ptr<Cmd>(new PrimeBuildCmd(from));
    case CMD_GIFT_CHAR:
        return boost::shared_ptr<Cmd>(new GiftCmd(from));
    case CMD_GATEWAYSCUTTLE_CHAR:
        return boost::shared_ptr<Cmd>(new GatewayScuttleCmd(from));
    case CMD_STOPSCUTTLE_CHAR:
        return boost::shared_ptr<Cmd>(new StopScuttleCmd(from));
    case CMD_WARPOUT_CHAR:
        return boost::shared_ptr<Cmd>(new WarpOutCmd(from));
    }
    return {};
}

AuthdCmd::AuthdCmd(boost::shared_ptr<Cmd> cmd, Address playerAddress)
    : cmd(cmd), playerAddress(playerAddress){}

uint8_t Cmd::getTypechar()
{
    throw runtime_error("getTypechar is not defined for '" + getTypename() + "'");
}
string Cmd::getTypename()
{
    throw runtime_error("getTypename is not defined for this command");
}
void Cmd::pack(Netpack::Builder* to)
{
    throw runtime_error("pack is not defined for the command '" + getTypename() + "'");
}

void Cmd::packCmdBasics(Netpack::Builder* to)
{
    packTypechar(to, getTypechar());
}

Cmd::Cmd() {}

Cmd::Cmd(Netpack::Consumer* from)
{}


uint8_t SpawnBeaconCmd::getTypechar()
{
    return CMD_SPAWNBEACON_CHAR;
}
string SpawnBeaconCmd::getTypename()
{
    return "SpawnBeaconCmd";
}
void SpawnBeaconCmd::executeAsPlayer(Game* game, Address playerAddress)
{
    optional<uint8_t> maybePlayerId = game->playerAddressToMaybeId(playerAddress);
    if (!maybePlayerId)
        return;
    
    uint8_t playerId = *maybePlayerId;

    if (game->getPlayerBeaconAvailable(playerId))
    {
        boost::shared_ptr<Beacon> beacon(new Beacon(&game->gameSettings, playerId, this->pos, Beacon::Spawning, true));
        if (game->registerNewEntityIfInMapAndNoCollision(beacon))
        {
            game->setPlayerBeaconAvailable(playerId, false);
        }
    }
}

SpawnBeaconCmd::SpawnBeaconCmd(vector2fp pos)
    : pos(pos)
{}
SpawnBeaconCmd::SpawnBeaconCmd(Netpack::Consumer* from)
    : Cmd(from)
{
    pos = consumeVector2fp(from);
}
void SpawnBeaconCmd::pack(Netpack::Builder* to)
{
    packCmdBasics(to);
    packVector2fp(to, pos);
}


uint8_t WithdrawCmd::getTypechar()
{
    return CMD_WITHDRAW_CHAR;
}
string WithdrawCmd::getTypename()
{
    return "WithdrawCmd";
}
void WithdrawCmd::pack(Netpack::Builder* to)
{
    this->packCmdBasics(to);
    packCoinsInt(to, amount);
}
WithdrawCmd::WithdrawCmd(coinsInt amount)
    : amount(amount)
{}
WithdrawCmd::WithdrawCmd(Netpack::Consumer* from)
    : Cmd(from)
{
    amount = consumeCoinsInt(from);
}


UnitCmd::UnitCmd(vector<EntityRef> unitRefs)
    : unitRefs(unitRefs)
{}

void UnitCmd::packUnitCmdBasics(Netpack::Builder* to)
{
    packCmdBasics(to);
    
    if (unitRefs.size() > 65535)
    {
        throw runtime_error("too many units referenced in that command!");
    }
    to->packUint16_t(unitRefs.size());
    for (unsigned i = 0; i < unitRefs.size(); i++)
    {
        packEntityRef(to, unitRefs[i]);
    }
}
UnitCmd::UnitCmd(Netpack::Consumer* from)
    : Cmd(from)
{
    uint16_t numUnitRefs = from->consumeUint16_t();
    for (unsigned i = 0; i < numUnitRefs; i++)
    {
        EntityRef unitRef = consumeEntityRef(from);
        unitRefs.push_back(unitRef);
    }
}

void UnitCmd::executeAsPlayer(Game *game, Address playerAddress)
{
    optional<uint8_t> maybePlayerId = game->playerAddressToMaybeId(playerAddress);
    if (!maybePlayerId)
        return;
    
    uint8_t playerId = *maybePlayerId;

    prepForUnitExecution(game, playerId);

    vector<boost::shared_ptr<Unit>> units = getUnits(game);
    for (unsigned int i = 0; i < units.size(); i++)
    {
        if (units[i]->ownerId == playerId)
        {
            executeOnUnit(units[i]);
        }
    }
}
void UnitCmd::prepForUnitExecution(Game* game, uint8_t playerId)
{}
void UnitCmd::executeOnUnit(boost::shared_ptr<Unit> u)
{
    throw runtime_error("executeOnUnit is not defined for the command '" + getTypename() + "'");
}

vector<boost::shared_ptr<Unit>> UnitCmd::getUnits(Game *game)
{
    vector<boost::shared_ptr<Unit>> units;
    for (unsigned int i = 0; i < unitRefs.size(); i++)
    {
        if (boost::shared_ptr<Unit> u = boost::dynamic_pointer_cast<Unit, Entity>(maybeEntityRefToPtrOrNull(*game, unitRefs[i])))
        {
            units.push_back(u);
        }
        else
        {
            cout << "NOTE: cmd contained a non-unit entity reference!" << endl;
        }
    }
    return units;
}


uint8_t MoveCmd::getTypechar()
{
    return CMD_MOVE_CHAR;
}
string MoveCmd::getTypename()
{
    return "MoveCmd";
}

void MoveCmd::executeOnUnit(boost::shared_ptr<Unit> unit)
{
    if (boost::shared_ptr<MobileUnit> mUnit = boost::dynamic_pointer_cast<MobileUnit, Unit>(unit))
    {
        mUnit->cmdMove(pos);
    }
    else
    {
        cout << "That's not a mobile unit!!" << endl;
    }
}

MoveCmd::MoveCmd(vector<EntityRef> units, vector2fp pos)
    : UnitCmd(units), pos(pos) {}
void MoveCmd::pack(Netpack::Builder* to)
{
    packUnitCmdBasics(to);
    packVector2fp(to, pos);
}
MoveCmd::MoveCmd(Netpack::Consumer* from)
    : UnitCmd(from)
{
    pos = consumeVector2fp(from);
}


uint8_t AttackScuttleCmd::getTypechar()
{
    return CMD_ATTACKSCUTTLE_CHAR;
}
string AttackScuttleCmd::getTypename()
{
    return "AttackScuttleCmd";
}

AttackScuttleCmd::AttackScuttleCmd(vector<EntityRef> units, Target target, bool asap)
    : UnitCmd(units), target(target), asap(asap)
{}
void AttackScuttleCmd::pack(Netpack::Builder* to)
{
    packUnitCmdBasics(to);
    target.pack(to);
    to->packBool(asap);
}
AttackScuttleCmd::AttackScuttleCmd(Netpack::Consumer* from)
    : UnitCmd(from), target((EntityRef)0)
{
    target = Target(from);
    asap = from->consumeBool();
}

void AttackScuttleCmd::executeOnUnit(boost::shared_ptr<Unit> unit)
{
    if (unit->getRefOrThrow() == target.castToEntityRef())
        return;

    if (auto targetEntity = target.castToEntityPtr(*unit->getGameOrThrow()))
    {
        if (getAllianceType(unit->ownerId, targetEntity) == Foreign)
        {
            if (auto combatUnit = boost::dynamic_pointer_cast<CombatUnit, Unit>(unit))
            {
                combatUnit->cmdAttack(target);
            }
            else
            {
                // do nothing
            };
        }
        else
        {
            if (auto prime = boost::dynamic_pointer_cast<Prime, Unit>(unit))
            {
                prime->cmdScuttle(targetEntity, asap);
            }
            else if (auto gateway = boost::dynamic_pointer_cast<Gateway, Unit>(unit))
            {
                gateway->cmdScuttle(targetEntity->getRefOrThrow());
            }
            else
            {
                // do nothing
            }
        }
    }
    else if (auto targetPoint = target.castToPoint())
    {
        if (auto combatUnit = boost::dynamic_pointer_cast<CombatUnit, Unit>(unit))
        {
            combatUnit->cmdAttack(target);
        }
        else if (auto prime = boost::dynamic_pointer_cast<Prime, Unit>(unit))
        {
            prime->cmdFetch(target, asap);
        }
    }
}

uint8_t StopCmd::getTypechar()
{
    return CMD_STOP_CHAR;
}
string StopCmd::getTypename()
{
    return "StopCmd";
}

void StopCmd::executeOnUnit(boost::shared_ptr<Unit> unit)
{
    unit->cmdStop();
}

StopCmd::StopCmd(vector<EntityRef> units)
    : UnitCmd(units) {}
void StopCmd::pack(Netpack::Builder* to)
{
    packUnitCmdBasics(to);
}
StopCmd::StopCmd(Netpack::Consumer* from)
    : UnitCmd(from) {}



uint8_t DepositCmd::getTypechar()
{
    return CMD_DEPOSIT_CHAR;
}
string DepositCmd::getTypename()
{
    return "DepositCmd";
}

DepositCmd::DepositCmd(vector<EntityRef> units, Target target, bool asap)
    : UnitCmd(units), target(target), asap(asap) {}
void DepositCmd::pack(Netpack::Builder* to)
{
    packUnitCmdBasics(to);
    target.pack(to);
    to->packBool(asap);
}
DepositCmd::DepositCmd(Netpack::Consumer* from)
    : UnitCmd(from), target((EntityRef)0)
{
    target = Target(from);
    asap = from->consumeBool();
}

void DepositCmd::prepForUnitExecution(Game* game, uint8_t ownerId)
{
    if (auto entity = target.castToEntityPtr(*game))
    {
        entityToDepositTo = entity;
    }
    else if (auto point = target.castToPoint())
    {
        entityToDepositTo = boost::shared_ptr<GoldPile>(new GoldPile(*point));

        if (! game->registerNewEntityIfInMapIgnoringCollision(entityToDepositTo))
        {
            entityToDepositTo.reset();
        }
    }
}
void DepositCmd::executeOnUnit(boost::shared_ptr<Unit> unit)
{
    if (!entityToDepositTo)
        return;
    
    if (unit->getRefOrThrow() == entityToDepositTo->getRefOrThrow())
        return;

    if (auto prime = boost::dynamic_pointer_cast<Prime, Unit>(unit))
    {
        prime->cmdDeposit(entityToDepositTo->getRefOrThrow(), asap);
    }

    else if (auto gateway = boost::dynamic_pointer_cast<Gateway, Unit>(unit))
    {
        gateway->cmdDepositTo(entityToDepositTo->getRefOrThrow());
    }
    else
        cout << "That's not a prime!!" << endl;
}



uint8_t FetchCmd::getTypechar()
{
    return CMD_FETCH_CHAR;
}
string FetchCmd::getTypename()
{
    return "FetchCmd";
}

FetchCmd::FetchCmd(vector<EntityRef> units, Target target, bool asap)
    : UnitCmd(units), target(target), asap(asap) {}
void FetchCmd::pack(Netpack::Builder* to)
{
    packUnitCmdBasics(to);
    target.pack(to);
    to->packBool(asap);
}
FetchCmd::FetchCmd(Netpack::Consumer* from)
    : UnitCmd(from), target(EntityRef(0))
{
    target = Target(from);
    asap = from->consumeBool();
}

void FetchCmd::executeOnUnit(boost::shared_ptr<Unit> unit)
{
    if (auto prime = boost::dynamic_pointer_cast<Prime, Unit>(unit))
    {
        prime->cmdFetch(target, asap);
    }
    else if (auto gateway = boost::dynamic_pointer_cast<Gateway, Unit>(unit))
    {
        if (auto entityRef = target.castToEntityRef())
        {
            gateway->cmdScuttle(*entityRef);
        }
    }
    else
    {
        cout << "That's not a Prime or Gateway!!" << endl;
    }
}


uint8_t GatewayBuildCmd::getTypechar()
{
    return CMD_GATEWAYBUILD_CHAR;
}
string GatewayBuildCmd::getTypename()
{
    return "GatewayBuildCmd";
}

void GatewayBuildCmd::executeOnUnit(boost::shared_ptr<Unit> unit)
{
    if (auto gateway = boost::dynamic_pointer_cast<Gateway, Unit>(unit))
    {
        gateway->cmdBuildUnit(buildTypechar);
    }
    else
    {
        cout << "trying to gatewayBuild on something other than a gateway!" << endl;
    }
}

GatewayBuildCmd::GatewayBuildCmd(vector<EntityRef> units, uint8_t buildTypechar)
    : UnitCmd(units), buildTypechar(buildTypechar) {}
void GatewayBuildCmd::pack(Netpack::Builder* to)
{
    packUnitCmdBasics(to);
    packTypechar(to, buildTypechar);
}
GatewayBuildCmd::GatewayBuildCmd(Netpack::Consumer* from)
    : UnitCmd(from)
{
    buildTypechar = consumeTypechar(from);
}

uint8_t PrimeBuildCmd::getTypechar()
{
    return CMD_PRIMEBUILD_CHAR;
}
string PrimeBuildCmd::getTypename()
{
    return "PrimeBuildCmd";
}

PrimeBuildCmd::PrimeBuildCmd(vector<EntityRef> units, uint8_t buildTypechar, vector2fp buildPos, bool asap)
    : UnitCmd(units), buildTypechar(buildTypechar), buildPos(buildPos), asap(asap) {}
void PrimeBuildCmd::pack(Netpack::Builder* to)
{
    packUnitCmdBasics(to);
    packTypechar(to, buildTypechar);
    packVector2fp(to, buildPos);
    to->packBool(asap);
}
PrimeBuildCmd::PrimeBuildCmd(Netpack::Consumer* from)
    : UnitCmd(from)
{
    buildTypechar = consumeTypechar(from);
    buildPos = consumeVector2fp(from);
    asap = from->consumeBool();
}

void PrimeBuildCmd::prepForUnitExecution(Game* game, uint8_t ownerId)
{
    switch (buildTypechar)
    {
        case GATEWAY_TYPECHAR:
            unitToBuild = boost::shared_ptr<Gateway>(new Gateway(&game->gameSettings, ownerId, buildPos));
            break;
        case TURRET_TYPECHAR:
            unitToBuild = boost::shared_ptr<Turret>(new Turret(&game->gameSettings, ownerId, buildPos));
            break;
        default:
            cout << "Prime doesn't know how to build that unit..." << endl;
            break;
    }
    if (unitToBuild)
    {
        if (! game->registerNewEntityIfInMapAndNoCollision(unitToBuild))
        {
            unitToBuild.reset();
        }
    }

}
void PrimeBuildCmd::executeOnUnit(boost::shared_ptr<Unit> unit)
{
    if (!unitToBuild)
        return;
    
    if (auto prime = boost::dynamic_pointer_cast<Prime, Unit>(unit))
    {
        prime->cmdDeposit(unitToBuild->getRefOrThrow(), asap);
    }
    else
    {
        cout << "trying to primeBuild on something other than a prime!" << endl;
    }
}

uint8_t GiftCmd::getTypechar()
{
    return CMD_GIFT_CHAR;
}
string GiftCmd::getTypename()
{
    return "GiftCmd";
}

void GiftCmd::executeOnUnit(boost::shared_ptr<Unit> unit)
{
    unit->ownerId = this->newOwnerId;
    // todo: is this all that's needed?
}

GiftCmd::GiftCmd(vector<EntityRef> units, uint8_t newOwnerId)
    : UnitCmd(units), newOwnerId(newOwnerId)
{}
void GiftCmd::pack(Netpack::Builder* to)
{
    packUnitCmdBasics(to);
    to->packUint8_t(newOwnerId);
}
GiftCmd::GiftCmd(Netpack::Consumer* from)
    : UnitCmd(from)
{
    newOwnerId = from->consumeUint8_t();
}


uint8_t GatewayScuttleCmd::getTypechar()
{
    return CMD_GATEWAYSCUTTLE_CHAR;
}
string GatewayScuttleCmd::getTypename()
{
    return "GatewayScuttleCmd";
}

void GatewayScuttleCmd::executeOnUnit(boost::shared_ptr<Unit> unit)
{
    if (auto gateway = boost::dynamic_pointer_cast<Gateway, Unit>(unit))
    {
        gateway->cmdScuttle(targetUnit);
    }
    else
    {
        cout << "Trying to execute a GatewayScuttle for a unit other than Gateway!" << endl;
    }
}

GatewayScuttleCmd::GatewayScuttleCmd(vector<EntityRef> units, EntityRef targetUnit)
    : UnitCmd(units), targetUnit(targetUnit)
{}
void GatewayScuttleCmd::pack(Netpack::Builder* to)
{
    packUnitCmdBasics(to);
    packEntityRef(to, targetUnit);
}
GatewayScuttleCmd::GatewayScuttleCmd(Netpack::Consumer* from)
    : UnitCmd(from)
{
    targetUnit = consumeEntityRef(from);
}

uint8_t StopScuttleCmd::getTypechar()
{
    return CMD_STOPSCUTTLE_CHAR;
}
string StopScuttleCmd::getTypename()
{
    return "StopScuttleCmd";
}

void StopScuttleCmd::executeOnUnit(boost::shared_ptr<Unit> unit)
{
    if (auto prime = boost::dynamic_pointer_cast<Prime, Unit>(unit))
    {
        // prime->cmdStopScuttle(targetUnit);
    }
    else if (auto gateway = boost::dynamic_pointer_cast<Gateway, Unit>(unit))
    {
        gateway->cmdStopScuttle(targetUnit);
    }
    else
    {
        cout << "Trying to call Scuttle for a unit other than Prime or Gateway!" << endl;
    }
}

StopScuttleCmd::StopScuttleCmd(vector<EntityRef> units, EntityRef targetUnit)
    : UnitCmd(units), targetUnit(targetUnit)
{}
void StopScuttleCmd::pack(Netpack::Builder* to)
{
    packUnitCmdBasics(to);
    packEntityRef(to, targetUnit);
}
StopScuttleCmd::StopScuttleCmd(Netpack::Consumer* from)
    : UnitCmd(from)
{
    targetUnit = consumeEntityRef(from);
}

uint8_t WarpOutCmd::getTypechar()
{
    return CMD_WARPOUT_CHAR;
}
string WarpOutCmd::getTypename()
{
    return "WarpOutCmd";
}

void WarpOutCmd::executeOnUnit(boost::shared_ptr<Unit> unit)
{
    if (auto gateway = boost::dynamic_pointer_cast<Gateway, Unit>(unit))
    {
        gateway->cmdScuttle(gateway->getRefOrThrow());
    }
    else if (auto beacon = boost::dynamic_pointer_cast<Beacon, Unit>(unit))
    {
        beacon->cmdWarpOut();
        if (beacon->getGameOrThrow()->mode == Game::GameMode::Pregame)
        {
            beacon->die();
        }
    }
    else
    {
        cout << "Trying to call Scuttle for a unit other than Prime or Gateway!" << endl;
    }
}

WarpOutCmd::WarpOutCmd(vector<EntityRef> units)
    : UnitCmd(units)
{}
void WarpOutCmd::pack(Netpack::Builder* to)
{
    packUnitCmdBasics(to);
}
WarpOutCmd::WarpOutCmd(Netpack::Consumer* from)
    : UnitCmd(from)
{}
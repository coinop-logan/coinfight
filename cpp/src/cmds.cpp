#include <stdexcept>
#include <boost/shared_ptr.hpp>
#include "cmds.h"
#include "common.h"

using namespace std;

boost::shared_ptr<Cmd> consumeCmd(Netpack::Consumer* from)
{
    uint8_t typechar = consumeTypechar(from);

    switch (typechar)
    {
    case CMD_MOVE_CHAR:
        return boost::shared_ptr<Cmd>(new MoveCmd(from));
    case CMD_PICKUP_CHAR:
        return boost::shared_ptr<Cmd>(new PickupCmd(from));
    case CMD_PUTDOWN_CHAR:
        return boost::shared_ptr<Cmd>(new PutdownCmd(from));
    case CMD_GATEWAYBUILD_CHAR:
        return boost::shared_ptr<Cmd>(new GatewayBuildCmd(from));
    case CMD_WITHDRAW_CHAR:
        return boost::shared_ptr<Cmd>(new WithdrawCmd(from));
    case CMD_ATTACK_CHAR:
        return boost::shared_ptr<Cmd>(new AttackGatherCmd(from));
    case CMD_PRIMEBUILD_CHAR:
        return boost::shared_ptr<Cmd>(new PrimeBuildCmd(from));
    case CMD_RESUMEBUILDING_CHAR:
        return boost::shared_ptr<Cmd>(new ResumeBuildingCmd(from));
    case CMD_SPAWNBEACON_CHAR:
        return boost::shared_ptr<Cmd>(new SpawnBeaconCmd(from));
    case CMD_SCUTTLE_CHAR:
        return boost::shared_ptr<Cmd>(new ScuttleCmd(from));
    }
    throw runtime_error("Trying to unpack an unrecognized cmd");
}

AuthdCmd::AuthdCmd(boost::shared_ptr<Cmd> cmd, string playerAddress)
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


uint8_t SpawnBeaconCmd::getTypechar()
{
    return CMD_SPAWNBEACON_CHAR;
}
string SpawnBeaconCmd::getTypename()
{
    return "SpawnBeaconCmd";
}
void SpawnBeaconCmd::executeAsPlayer(Game* game, string playerAddress)
{
    optional<uint8_t> maybePlayerId = game->playerAddressToMaybeId(playerAddress);
    if (!maybePlayerId)
        return;
    
    uint8_t playerId = *maybePlayerId;

    if (game->getPlayerBeaconAvailable(playerId))
    {
        boost::shared_ptr<Beacon> beacon(new Beacon(playerId, this->pos, Beacon::Spawning));
        if (game->registerNewEntityIfNoCollision(beacon)) // will fail in case of collision with other unit
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

void UnitCmd::executeAsPlayer(Game *game, string playerAddress)
{
    optional<uint8_t> maybePlayerId = game->playerAddressToMaybeId(playerAddress);
    if (!maybePlayerId)
        return;
    
    uint8_t playerId = *maybePlayerId;

    vector<boost::shared_ptr<Unit>> units = getUnits(game);
    for (unsigned int i = 0; i < units.size(); i++)
    {
        if (units[i]->ownerId == playerId)
        {
            executeOnUnit(units[i]);
        }
    }
}
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

uint8_t PickupCmd::getTypechar()
{
    return CMD_PICKUP_CHAR;
}
string PickupCmd::getTypename()
{
    return "PickupCmd";
}
void PickupCmd::executeOnUnit(boost::shared_ptr<Unit> unit)
{
    if (boost::shared_ptr<Prime> prime = boost::dynamic_pointer_cast<Prime, Unit>(unit))
    {
        prime->cmdPickup(goldRef);
    }
    else
    {
        cout << "That's not a Prime!!" << endl;
    }
}

PickupCmd::PickupCmd(vector<EntityRef> units, EntityRef goldRef)
    : UnitCmd(units), goldRef(goldRef) {}
void PickupCmd::pack(Netpack::Builder* to)
{
    packUnitCmdBasics(to);
    packEntityRef(to, goldRef);
}
PickupCmd::PickupCmd(Netpack::Consumer* from)
    : UnitCmd(from)
{
    goldRef = consumeEntityRef(from);
}

uint8_t PutdownCmd::getTypechar()
{
    return CMD_PUTDOWN_CHAR;
}
string PutdownCmd::getTypename()
{
    return "PutdownCmd";
}

void PutdownCmd::executeOnUnit(boost::shared_ptr<Unit> unit)
{
    if (auto prime = boost::dynamic_pointer_cast<Prime, Unit>(unit))
        prime->cmdPutdown(target);
    else if (auto gateway = boost::dynamic_pointer_cast<Gateway, Unit>(unit))
    {
        gateway->cmdDepositTo(target);
    }
    else
        cout << "That's not a prime!!" << endl;
}

PutdownCmd::PutdownCmd(vector<EntityRef> units, Target target)
    : UnitCmd(units), target(target) {}
void PutdownCmd::pack(Netpack::Builder* to)
{
    packUnitCmdBasics(to);
    target.pack(to);
}
PutdownCmd::PutdownCmd(Netpack::Consumer* from)
    : UnitCmd(from), target((EntityRef)0)
{
    target = Target(from);
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

void PrimeBuildCmd::executeOnUnit(boost::shared_ptr<Unit> unit)
{
    if (auto prime = boost::dynamic_pointer_cast<Prime, Unit>(unit))
    {
        prime->cmdBuild(buildTypechar, buildPos);
    }
    else
    {
        cout << "trying to primeBuild on something other than a prime!" << endl;
    }
}

PrimeBuildCmd::PrimeBuildCmd(vector<EntityRef> units, uint8_t buildTypechar, vector2fp buildPos)
    : UnitCmd(units), buildTypechar(buildTypechar), buildPos(buildPos) {}
void PrimeBuildCmd::pack(Netpack::Builder* to)
{
    packUnitCmdBasics(to);
    packTypechar(to, buildTypechar);
    packVector2fp(to, buildPos);
}
PrimeBuildCmd::PrimeBuildCmd(Netpack::Consumer* from)
    : UnitCmd(from)
{
    buildTypechar = consumeTypechar(from);
    buildPos = consumeVector2fp(from);
}

uint8_t AttackGatherCmd::getTypechar()
{
    return CMD_ATTACK_CHAR;
}
string AttackGatherCmd::getTypename()
{
    return "AttackGatherCmd";
}

void AttackGatherCmd::executeOnUnit(boost::shared_ptr<Unit> unit)
{
    if (auto fighter = boost::dynamic_pointer_cast<Fighter, Entity>(unit))
    {
        fighter->cmdAttack(target);
    }
    else if (auto prime = boost::dynamic_pointer_cast<Prime, Entity>(unit))
    {
        if (auto targetPoint = target.getPointUnlessTargetDeleted(*prime->getGameOrThrow()))
        {
            prime->cmdGather(*targetPoint);
        }
    }
}

AttackGatherCmd::AttackGatherCmd(vector<EntityRef> units, Target target)
    : UnitCmd(units), target(target)
{}
void AttackGatherCmd::pack(Netpack::Builder* to)
{
    packUnitCmdBasics(to);
    target.pack(to);
}
AttackGatherCmd::AttackGatherCmd(Netpack::Consumer* from)
    : UnitCmd(from), target((EntityRef)0)
{
    target = Target(from);
}

uint8_t ResumeBuildingCmd::getTypechar()
{
    return CMD_RESUMEBUILDING_CHAR;
}
string ResumeBuildingCmd::getTypename()
{
    return "ResumeBuildingCmd";
}

void ResumeBuildingCmd::executeOnUnit(boost::shared_ptr<Unit> unit)
{
    if (auto prime = boost::dynamic_pointer_cast<Prime, Entity>(unit))
    {
        prime->cmdResumeBuilding(targetUnit);
    }
}

ResumeBuildingCmd::ResumeBuildingCmd(vector<EntityRef> units, EntityRef targetUnit)
    : UnitCmd(units), targetUnit(targetUnit)
{}
void ResumeBuildingCmd::pack(Netpack::Builder* to)
{
    packUnitCmdBasics(to);
    packEntityRef(to, targetUnit);
}
ResumeBuildingCmd::ResumeBuildingCmd(Netpack::Consumer* from)
    : UnitCmd(from)
{
    targetUnit = consumeEntityRef(from);
}

uint8_t ScuttleCmd::getTypechar()
{
    return CMD_SCUTTLE_CHAR;
}
string ScuttleCmd::getTypename()
{
    return "ScuttleCmd";
}

void ScuttleCmd::executeOnUnit(boost::shared_ptr<Unit> unit)
{
    if (auto prime = boost::dynamic_pointer_cast<Prime, Unit>(unit))
    {
        prime->cmdScuttle(targetUnit);
    }
    else if (auto gateway = boost::dynamic_pointer_cast<Gateway, Unit>(unit))
    {
        gateway->cmdScuttle(targetUnit);
    }
    else
    {
        cout << "Trying to call Scuttle for a unit other than Prime or Gateway!" << endl;
    }
}

ScuttleCmd::ScuttleCmd(vector<EntityRef> units, EntityRef targetUnit)
    : UnitCmd(units), targetUnit(targetUnit)
{}
void ScuttleCmd::pack(Netpack::Builder* to)
{
    packUnitCmdBasics(to);
    packEntityRef(to, targetUnit);
}
ScuttleCmd::ScuttleCmd(Netpack::Consumer* from)
    : UnitCmd(from)
{
    targetUnit = consumeEntityRef(from);
}

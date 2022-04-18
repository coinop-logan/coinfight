#include <stdexcept>
#include <boost/shared_ptr.hpp>
#include "cmds.h"
#include "common.h"

using namespace std;

boost::shared_ptr<Cmd> unpackFullCmdAndMoveIter(vchIter *iter)
{
    unsigned char typechar;
    *iter = unpackTypecharFromIter(*iter, &typechar);

    switch (typechar)
    {
    case CMD_MOVE_CHAR:
        return boost::shared_ptr<Cmd>(new MoveCmd(iter));
    case CMD_PICKUP_CHAR:
        return boost::shared_ptr<Cmd>(new PickupCmd(iter));
    case CMD_PUTDOWN_CHAR:
        return boost::shared_ptr<Cmd>(new PutdownCmd(iter));
    case CMD_GATEWAYBUILD_CHAR:
        return boost::shared_ptr<Cmd>(new GatewayBuildCmd(iter));
    case CMD_WITHDRAW_CHAR:
        return boost::shared_ptr<Cmd>(new WithdrawCmd(iter));
    case CMD_ATTACK_CHAR:
        return boost::shared_ptr<Cmd>(new AttackCmd(iter));
    case CMD_PRIMEBUILD_CHAR:
        return boost::shared_ptr<Cmd>(new PrimeBuildCmd(iter));
    case CMD_RESUMEBUILDING_CHAR:
        return boost::shared_ptr<Cmd>(new ResumeBuildingCmd(iter));
    case CMD_SPAWNBEACON_CHAR:
        return boost::shared_ptr<Cmd>(new SpawnBeaconCmd(iter));
    case CMD_SCUTTLE_CHAR:
        return boost::shared_ptr<Cmd>(new ScuttleCmd(iter));
    }
    throw runtime_error("Trying to unpack an unrecognized cmd");
}

AuthdCmd::AuthdCmd(boost::shared_ptr<Cmd> cmd, string playerAddress)
    : cmd(cmd), playerAddress(playerAddress){}

unsigned char Cmd::getTypechar()
{
    throw runtime_error("getTypechar is not defined for '" + getTypename() + "'");
}
string Cmd::getTypename()
{
    throw runtime_error("getTypename is not defined for this command");
}
void Cmd::pack(vch *dest)
{
    throw runtime_error("pack is not defined for the command '" + getTypename() + "'");
}
void Cmd::unpackAndMoveIter(vchIter *iter)
{
    throw runtime_error("unpackAndMoveIter is not defined for the command '" + getTypename() + "'");
}

void Cmd::packCmd(vch *dest) {}

void Cmd::unpackCmdAndMoveIter(vchIter *iter) {}

Cmd::Cmd() {}

Cmd::Cmd(vchIter *iter)
{
    unpackCmdAndMoveIter(iter);
}


unsigned char WithdrawCmd::getTypechar()
{
    return CMD_WITHDRAW_CHAR;
}
string WithdrawCmd::getTypename()
{
    return "WithdrawCmd";
}
void WithdrawCmd::pack(vch *dest)
{
    packCmd(dest);
    packToVch(dest, "L", amount);
}
void WithdrawCmd::unpackAndMoveIter(vchIter *iter)
{
    *iter = unpackFromIter(*iter, "L", &amount);
}
WithdrawCmd::WithdrawCmd(coinsInt amount)
    : amount(amount)
{}
WithdrawCmd::WithdrawCmd(vchIter *iter)
{
    unpackAndMoveIter(iter);
}


unsigned char SpawnBeaconCmd::getTypechar()
{
    return CMD_SPAWNBEACON_CHAR;
}
string SpawnBeaconCmd::getTypename()
{
    return "SpawnBeaconCmd";
}
void SpawnBeaconCmd::executeAsPlayer(Game* game, string playerAddress)
{
    int playerId = game->playerAddressToIdOrNegativeOne(playerAddress);
    if (playerId == -1)
        return;

    if (game->getPlayerBeaconAvailable(playerId))
    {
        game->setPlayerBeaconAvailable(playerId, false);
        boost::shared_ptr<Beacon> beacon(new Beacon(game, game->getNextEntityRef(), playerId, this->pos, Beacon::Spawning));
        game->entities.push_back(beacon);
    }
}
void SpawnBeaconCmd::pack(vch *dest)
{
    packCmd(dest);
    packVector2f(dest, pos);
}
void SpawnBeaconCmd::unpackAndMoveIter(vchIter *iter)
{
    *iter = unpackVector2f(*iter, &pos);
}
SpawnBeaconCmd::SpawnBeaconCmd(vector2f pos)
    : pos(pos)
{}
SpawnBeaconCmd::SpawnBeaconCmd(vchIter *iter)
{
    unpackAndMoveIter(iter);
}


UnitCmd::UnitCmd(vector<EntityRef> unitRefs) : unitRefs(unitRefs) {}
UnitCmd::UnitCmd(vchIter *iter)
{
    unpackUnitCmdAndMoveIter(iter);
}
void UnitCmd::packUnitCmd(vch *dest)
{
    if (unitRefs.size() > 65535)
    {
        throw runtime_error("too many units referenced in that command!");
    }
    uint16_t numUnitRefs = unitRefs.size();
    packToVch(dest, "H", numUnitRefs);
    for (unsigned i = 0; i < numUnitRefs; i++)
    {
        packEntityRef(dest, unitRefs[i]);
    }
}
void UnitCmd::unpackUnitCmdAndMoveIter(vchIter *iter)
{
    uint16_t numUnitRefs;
    *iter = unpackFromIter(*iter, "H", &numUnitRefs);
    for (unsigned i = 0; i < numUnitRefs; i++)
    {
        EntityRef unitRef;
        *iter = unpackEntityRef(*iter, &unitRef);
        unitRefs.push_back(unitRef);
    }
}

void UnitCmd::executeAsPlayer(Game *game, string playerAddress)
{
    int playerId = game->playerAddressToIdOrNegativeOne(playerAddress);
    if (playerId == -1)
        return;

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
        if (boost::shared_ptr<Unit> u = boost::dynamic_pointer_cast<Unit, Entity>(entityRefToPtrOrNull(*game, unitRefs[i])))
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

unsigned char MoveCmd::getTypechar()
{
    return CMD_MOVE_CHAR;
}
string MoveCmd::getTypename()
{
    return "MoveCmd";
}
void MoveCmd::pack(vch *dest)
{
    packUnitCmd(dest);
    packVector2f(dest, pos);
}
void MoveCmd::unpackAndMoveIter(vchIter *iter)
{
    *iter = unpackVector2f(*iter, &pos);
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

MoveCmd::MoveCmd(vector<EntityRef> units, vector2f pos) : UnitCmd(units), pos(pos) {}
MoveCmd::MoveCmd(vchIter *iter) : UnitCmd(iter)
{
    unpackAndMoveIter(iter);
}

unsigned char PickupCmd::getTypechar()
{
    return CMD_PICKUP_CHAR;
}
string PickupCmd::getTypename()
{
    return "PickupCmd";
}
void PickupCmd::pack(vch *dest)
{
    packUnitCmd(dest);
    packEntityRef(dest, goldRef);
}
void PickupCmd::unpackAndMoveIter(vchIter *iter)
{
    *iter = unpackEntityRef(*iter, &goldRef);
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

PickupCmd::PickupCmd(vector<EntityRef> units, EntityRef goldRef) : UnitCmd(units), goldRef(goldRef) {}
PickupCmd::PickupCmd(vchIter *iter) : UnitCmd(iter)
{
    unpackAndMoveIter(iter);
}

unsigned char PutdownCmd::getTypechar()
{
    return CMD_PUTDOWN_CHAR;
}
string PutdownCmd::getTypename()
{
    return "PutdownCmd";
}
void PutdownCmd::pack(vch *dest)
{
    packUnitCmd(dest);
    target.pack(dest);
}
void PutdownCmd::unpackAndMoveIter(vchIter *iter)
{
    target = Target(iter);
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

PutdownCmd::PutdownCmd(vector<EntityRef> units, Target target) : UnitCmd(units), target(target) {}
PutdownCmd::PutdownCmd(vchIter *iter) : UnitCmd(iter), target(NULL_ENTITYREF)
{
    unpackAndMoveIter(iter);
}

unsigned char GatewayBuildCmd::getTypechar()
{
    return CMD_GATEWAYBUILD_CHAR;
}
string GatewayBuildCmd::getTypename()
{
    return "GatewayBuildCmd";
}
void GatewayBuildCmd::pack(vch *dest)
{
    packUnitCmd(dest);
    packTypechar(dest, buildTypechar);
}
void GatewayBuildCmd::unpackAndMoveIter(vchIter *iter)
{
    *iter = unpackTypecharFromIter(*iter, &buildTypechar);
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

GatewayBuildCmd::GatewayBuildCmd(vector<EntityRef> units, unsigned char buildTypechar)
    : UnitCmd(units), buildTypechar(buildTypechar) {}

GatewayBuildCmd::GatewayBuildCmd(vchIter *iter)
    : UnitCmd(iter)
{
    unpackAndMoveIter(iter);
}

unsigned char PrimeBuildCmd::getTypechar()
{
    return CMD_PRIMEBUILD_CHAR;
}
string PrimeBuildCmd::getTypename()
{
    return "PrimeBuildCmd";
}
void PrimeBuildCmd::pack(vch *dest)
{
    packUnitCmd(dest);
    packTypechar(dest, buildTypechar);
    packVector2f(dest, buildPos);
}
void PrimeBuildCmd::unpackAndMoveIter(vchIter *iter)
{
    *iter = unpackTypecharFromIter(*iter, &buildTypechar);
    *iter = unpackVector2f(*iter, &buildPos);
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

PrimeBuildCmd::PrimeBuildCmd(vector<EntityRef> units, unsigned char buildTypechar, vector2f buildPos)
    : UnitCmd(units), buildTypechar(buildTypechar), buildPos(buildPos) {}

PrimeBuildCmd::PrimeBuildCmd(vchIter *iter)
    : UnitCmd(iter)
{
    unpackAndMoveIter(iter);
}

unsigned char AttackCmd::getTypechar()
{
    return CMD_ATTACK_CHAR;
}
string AttackCmd::getTypename()
{
    return "AttackCmd";
}
void AttackCmd::pack(vch *dest)
{
    packUnitCmd(dest);
    packEntityRef(dest, targetUnit);
}
void AttackCmd::unpackAndMoveIter(vchIter *iter)
{
    *iter = unpackEntityRef(*iter, &targetUnit);
}

void AttackCmd::executeOnUnit(boost::shared_ptr<Unit> unit)
{
    if (auto fighter = boost::dynamic_pointer_cast<Fighter, Entity>(unit))
    {
        fighter->cmdAttack(targetUnit);
    }
}

AttackCmd::AttackCmd(vector<EntityRef> units, EntityRef targetUnit)
    : UnitCmd(units), targetUnit(targetUnit)
{}
AttackCmd::AttackCmd(vchIter *iter)
    : UnitCmd(iter)
{
    unpackAndMoveIter(iter);
}

unsigned char ResumeBuildingCmd::getTypechar()
{
    return CMD_RESUMEBUILDING_CHAR;
}
string ResumeBuildingCmd::getTypename()
{
    return "ResumeBuildingCmd";
}
void ResumeBuildingCmd::pack(vch *dest)
{
    packUnitCmd(dest);
    packEntityRef(dest, targetUnit);
}
void ResumeBuildingCmd::unpackAndMoveIter(vchIter *iter)
{
    *iter = unpackEntityRef(*iter, &targetUnit);
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
ResumeBuildingCmd::ResumeBuildingCmd(vchIter *iter)
    : UnitCmd(iter)
{
    unpackAndMoveIter(iter);
}

unsigned char ScuttleCmd::getTypechar()
{
    return CMD_SCUTTLE_CHAR;
}
string ScuttleCmd::getTypename()
{
    return "ScuttleCmd";
}
void ScuttleCmd::pack(vch *dest)
{
    packUnitCmd(dest);
    packEntityRef(dest, targetUnit);
}
void ScuttleCmd::unpackAndMoveIter(vchIter *iter)
{
    *iter = unpackEntityRef(*iter, &targetUnit);
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
ScuttleCmd::ScuttleCmd(vchIter *iter)
    : UnitCmd(iter)
{
    unpackAndMoveIter(iter);
}

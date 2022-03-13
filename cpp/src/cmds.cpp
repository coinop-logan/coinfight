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
    default:
        throw runtime_error("Trying to unpack an unrecognized cmd");
    }
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

void UnitCmd::executeAsPlayer(Game *game, string userAddress)
{
    int playerId = game->playerAddressToIdOrNegativeOne(userAddress);
    if (playerId == -1)
        return;
        
    vector<boost::shared_ptr<Unit>> units = getUnits(game);
    for (uint i = 0; i < units.size(); i++)
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
    for (uint i = 0; i < unitRefs.size(); i++)
    {
        if (boost::shared_ptr<Unit> u = boost::dynamic_pointer_cast<Unit, Entity>(entityRefToPtrOrNull(*game, unitRefs[i])))
        {
            units.push_back(u);
        }
        else
        {
            cout << "WARNING: cmd contained a non-unit entity reference!" << endl;
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
    if (boost::shared_ptr<Prime> prime = boost::dynamic_pointer_cast<Prime, Unit>(unit))
        prime->cmdPutdown(target);
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
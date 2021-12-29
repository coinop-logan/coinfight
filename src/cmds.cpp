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
    case CMD_SENDGOLDTHROUGHGATEWAY_CHAR:
        return boost::shared_ptr<Cmd>(new SendGoldThroughGatewayCmd(iter));
    default:
        throw runtime_error("Trying to unpack an unrecognized cmd");
    }
}

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
void Cmd::executeOnUnit(boost::shared_ptr<Unit> u)
{
    throw runtime_error("executeOnUnit is not defined for the command '" + getTypename() + "'");
}

vector<boost::shared_ptr<Unit>> Cmd::getUnits(Game *game)
{
    vector<boost::shared_ptr<Unit>> units;
    for (uint i = 0; i < unitRefs.size(); i++)
    {
        if (boost::shared_ptr<Unit> u = boost::dynamic_pointer_cast<Unit, Entity>(game->entityRefToPtr(unitRefs[i])))
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
void Cmd::execute(Game *game)
{
    vector<boost::shared_ptr<Unit>> units = getUnits(game);
    for (uint i = 0; i < units.size(); i++)
    {
        executeOnUnit(units[i]);
    }
}

void Cmd::packCmd(vch *dest)
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
void Cmd::unpackCmdAndMoveIter(vchIter *iter)
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

Cmd::Cmd(vector<EntityRef> unitRefs) : unitRefs(unitRefs) {}

Cmd::Cmd(vchIter *iter)
{
    unpackCmdAndMoveIter(iter);
}

void FrameCmdsPacket::pack(vch *dest)
{
    packToVch(dest, "QC", frame, (unsigned char)(cmds.size()));

    for (unsigned int i = 0; i < cmds.size(); i++)
    {
        packTypechar(dest, cmds[i]->getTypechar());
        cmds[i]->pack(dest);
    }
}

void FrameCmdsPacket::unpackAndMoveIter(vchIter *iter)
{
    unsigned char numCmds;
    *iter = unpackFromIter(*iter, "QC", &frame, &numCmds);

    cmds.clear();
    for (unsigned int i = 0; i < numCmds; i++)
    {
        cmds.push_back(unpackFullCmdAndMoveIter(iter));
    }
}

FrameCmdsPacket::FrameCmdsPacket(uint64_t frame, vector<boost::shared_ptr<Cmd>> cmds)
    : frame(frame), cmds(cmds) {}

FrameCmdsPacket::FrameCmdsPacket(vchIter *iter)
{
    unpackAndMoveIter(iter);
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
    packCmd(dest);
    packVector2f(dest, pos);
}
void MoveCmd::unpackAndMoveIter(vchIter *iter)
{
    *iter = unpackVector2f(*iter, &pos);
}

void MoveCmd::executeOnUnit(boost::shared_ptr<Unit> unit)
{
    if (!unit->isActive())
    {
        return;
    }

    if (boost::shared_ptr<MobileUnit> mUnit = boost::dynamic_pointer_cast<MobileUnit, Unit>(unit))
    {
        mUnit->cmdMove(pos);
    }
    else
    {
        cout << "That's not a mobile unit!!" << endl;
    }
}

MoveCmd::MoveCmd(vector<EntityRef> units, vector2f pos) : Cmd(units), pos(pos) {}
MoveCmd::MoveCmd(vchIter *iter) : Cmd(iter)
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
    packCmd(dest);
    packEntityRef(dest, goldRef);
}
void PickupCmd::unpackAndMoveIter(vchIter *iter)
{
    *iter = unpackEntityRef(*iter, &goldRef);
}
void PickupCmd::executeOnUnit(boost::shared_ptr<Unit> unit)
{
    if (!unit->isActive())
    {
        return;
    }

    if (boost::shared_ptr<Prime> prime = boost::dynamic_pointer_cast<Prime, Unit>(unit))
    {
        prime->cmdPickup(goldRef);
    }
    else
    {
        cout << "That's not a Prime!!" << endl;
    }
}

PickupCmd::PickupCmd(vector<EntityRef> units, EntityRef goldRef) : Cmd(units), goldRef(goldRef) {}
PickupCmd::PickupCmd(vchIter *iter) : Cmd(iter)
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
    packCmd(dest);
    target.pack(dest);
}
void PutdownCmd::unpackAndMoveIter(vchIter *iter)
{
    target = Target(iter);
}

void PutdownCmd::executeOnUnit(boost::shared_ptr<Unit> unit)
{
    if (!unit->isActive())
        return;

    if (boost::shared_ptr<Prime> prime = boost::dynamic_pointer_cast<Prime, Unit>(unit))
        prime->cmdPutdown(target);
    else
        cout << "That's not a prime!!" << endl;
}

PutdownCmd::PutdownCmd(vector<EntityRef> units, Target target) : Cmd(units), target(target) {}
PutdownCmd::PutdownCmd(vchIter *iter) : Cmd(iter), target(NULL_ENTITYREF)
{
    unpackAndMoveIter(iter);
}

unsigned char SendGoldThroughGatewayCmd::getTypechar()
{
    return CMD_SENDGOLDTHROUGHGATEWAY_CHAR;
}
string SendGoldThroughGatewayCmd::getTypename()
{
    return "SendGoldThroughGatewayCmd";
}
void SendGoldThroughGatewayCmd::pack(vch *dest)
{
    packCmd(dest);
    packEntityRef(dest, gatewayRef);
}
void SendGoldThroughGatewayCmd::unpackAndMoveIter(vchIter *iter)
{
    *iter = unpackEntityRef(*iter, &gatewayRef);
}

void SendGoldThroughGatewayCmd::executeOnUnit(boost::shared_ptr<Unit> unit)
{
    if (!unit->isActive())
        return;
    
    boost::shared_ptr<Entity> gatewayEntity = unit->game->entityRefToPtr(gatewayRef);
    if (boost::shared_ptr<Prime> prime = boost::dynamic_pointer_cast<Prime, Unit>(unit))
        if (boost::shared_ptr<Gateway> gateway = boost::dynamic_pointer_cast<Gateway, Entity>(gatewayEntity))
        {
            prime->cmdSendGoldThroughGateway(gateway);
        }
}

SendGoldThroughGatewayCmd::SendGoldThroughGatewayCmd(vector<EntityRef> units, EntityRef gatewayRef) : Cmd(units), gatewayRef(gatewayRef){}
SendGoldThroughGatewayCmd::SendGoldThroughGatewayCmd(vchIter *iter) : Cmd(iter), gatewayRef(NULL_ENTITYREF)
{
    unpackAndMoveIter(iter);
}
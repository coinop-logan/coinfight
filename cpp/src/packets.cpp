#include "packets.h"

unsigned char Packet::typechar()
{
    throw runtime_error("typechar() has not been defined for a packet type.");
}

void Packet::pack(vch *dest)
{
    throw runtime_error("pack() has not been defined for a packet");
}
void Packet::unpackAndMoveIter(vchIter *iter)
{
    throw runtime_error("unpackMoveIter() has not been defined for a packet");
}

void Packet::packPacket(vch *destVch) {}
void Packet::unpackPacketAndMoveIter(vchIter *iter) {}

Packet::Packet() {}
Packet::Packet(vchIter *iter)
{
    unpackPacketAndMoveIter(iter);
}

unsigned char FrameEventsPacket::typechar()
{
    return PACKET_FRAMECMDS_CHAR;
}
void FrameEventsPacket::pack(vch *dest)
{
    packPacket(dest);

    packToVch(dest, "QCC", frame, (unsigned char)(cmds.size()), (unsigned char)(balanceUpdates.size()));

    for (unsigned int i = 0; i < cmds.size(); i++)
    {
        packTypechar(dest, cmds[i]->getTypechar());
        cmds[i]->pack(dest);
    }

    for (unsigned int i = 0; i < balanceUpdates.size(); i++)
    {
        balanceUpdates[i]->pack(dest);
    }
}

void FrameEventsPacket::unpackAndMoveIter(vchIter *iter)
{
    unsigned char numCmds, numBalanceUpdates;
    *iter = unpackFromIter(*iter, "QCC", &frame, &numCmds, &numBalanceUpdates);

    cmds.clear();
    for (unsigned int i = 0; i < numCmds; i++)
    {
        cmds.push_back(unpackFullCmdAndMoveIter(iter));
    }

    balanceUpdates.clear();
    for (unsigned int i = 0; i < numBalanceUpdates; i++)
    {
        balanceUpdates.push_back(boost::shared_ptr<BalanceUpdate>(new BalanceUpdate(iter)));
    }
}

FrameEventsPacket::FrameEventsPacket(uint64_t frame, vector<boost::shared_ptr<Cmd>> cmds, vector<boost::shared_ptr<BalanceUpdate>> balanceUpdates)
    : frame(frame), cmds(cmds), balanceUpdates(balanceUpdates) {}

FrameEventsPacket::FrameEventsPacket(vchIter *iter)
{
    unpackAndMoveIter(iter);
}
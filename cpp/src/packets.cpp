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

unsigned char FrameCmdsPacket::typechar()
{
    return PACKET_FRAMECMDS_CHAR;
}
void FrameCmdsPacket::pack(vch *dest)
{
    packPacket(dest);

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

unsigned char BalanceUpdatePacket::typechar()
{
    return PACKET_BALANCEUPDATE_CHAR;
}

void BalanceUpdatePacket::pack(vch *dest)
{
    packPacket(dest);

    packStringToVch(dest, userAddress);
    packToVch(dest, "L", newBalance);
}
void BalanceUpdatePacket::unpackAndMoveIter(vchIter *iter)
{
    // *iter = unpackStringFromIter(*iter, )
}

BalanceUpdatePacket::BalanceUpdatePacket(string userAddress, coinsInt newBalance)
{

}
BalanceUpdatePacket::BalanceUpdatePacket(vchIter *iter)
{

}
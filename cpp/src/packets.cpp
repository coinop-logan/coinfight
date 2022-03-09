#include "packets.h"
#include "events.h"

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

    packToVch(dest, "QCC", frame, (unsigned char)(authdCmds.size()), (unsigned char)(events.size()));

    for (unsigned int i = 0; i < authdCmds.size(); i++)
    {
        packStringToVch(dest, authdCmds[i]->playerAddress);
        packTypechar(dest, authdCmds[i]->cmd->getTypechar());
        authdCmds[i]->cmd->pack(dest);
    }

    for (unsigned int i = 0; i < events.size(); i++)
    {
        packTypechar(dest, events[i]->typechar());
        events[i]->pack(dest);
    }
}

void FrameEventsPacket::unpackAndMoveIter(vchIter *iter)
{
    unsigned char numCmds, numEvents;
    *iter = unpackFromIter(*iter, "QCC", &frame, &numCmds, &numEvents);

    authdCmds.clear();
    for (unsigned int i = 0; i < numCmds; i++)
    {
        string playerAddress;
        *iter = unpackStringFromIter(*iter, 42, &playerAddress);
        boost::shared_ptr<Cmd> unauthdCmd = unpackFullCmdAndMoveIter(iter);

        authdCmds.push_back(boost::shared_ptr<AuthdCmd>(new AuthdCmd(unauthdCmd, playerAddress)));
    }

    events.clear();
    for (unsigned int i = 0; i < numEvents; i++)
    {
        events.push_back(unpackFullEventAndMoveIter(iter));
    }
}

FrameEventsPacket::FrameEventsPacket(uint64_t frame, vector<boost::shared_ptr<AuthdCmd>> authdCmds, vector<boost::shared_ptr<Event>> events)
    : frame(frame), authdCmds(authdCmds), events(events) {}

FrameEventsPacket::FrameEventsPacket(vchIter *iter)
{
    unpackAndMoveIter(iter);
}
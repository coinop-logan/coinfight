#include "packets.h"
#include "events.h"

uint8_t Packet::typechar()
{
    throw runtime_error("typechar() has not been defined for a packet type.");
}

void Packet::pack(Netpack::Builder* to)
{
    throw runtime_error("pack() has not been defined for a packet");
}

void Packet::packPacketBasics(Netpack::Builder* to)
{
    packTypechar(to, typechar());
}

Packet::Packet() {}
Packet::Packet(Netpack::Consumer* from) {}

uint8_t FrameEventsPacket::typechar()
{
    return PACKET_FRAMECMDS_CHAR;
}

FrameEventsPacket::FrameEventsPacket(uint64_t frame, vector<boost::shared_ptr<AuthdCmd>> authdCmds, vector<boost::shared_ptr<Event>> events)
    : frame(frame), authdCmds(authdCmds), events(events) {}
void FrameEventsPacket::pack(Netpack::Builder* to)
{
    packPacketBasics(to);

    to->packUint64_t(frame);
    to->packUint16_t(authdCmds.size());
    to->packUint16_t(events.size());

    for (unsigned int i = 0; i < authdCmds.size(); i++)
    {
        authdCmds[i]->playerAddress.pack(to);
        authdCmds[i]->cmd->pack(to);
    }

    for (unsigned int i = 0; i < events.size(); i++)
    {
        events[i]->pack(to);
    }
}
FrameEventsPacket::FrameEventsPacket(Netpack::Consumer* from)
{
    frame = from->consumeUint64_t();
    uint16_t numCmds = from->consumeUint16_t();
    uint16_t numEvents = from->consumeUint16_t();

    authdCmds.clear();
    for (unsigned int i = 0; i < numCmds; i++)
    {
        Address playerAddress(from);
        boost::shared_ptr<Cmd> unauthdCmd = consumeCmd(from);
        if (!unauthdCmd)
        {
            throw runtime_error("Unrecognized command in FEP");
        }

        authdCmds.push_back(boost::shared_ptr<AuthdCmd>(new AuthdCmd(unauthdCmd, playerAddress)));
    }

    events.clear();
    for (unsigned int i = 0; i < numEvents; i++)
    {
        optional<boost::shared_ptr<Event>> maybeEvent = consumeEvent(from);
        if (!maybeEvent)
        {
            throw runtime_error("Trying to unpack an unrecognized event");
        }
        events.push_back(*maybeEvent);
    }
}

void clearVchAndBuildCmdPacket(vch *dest, boost::shared_ptr<Cmd> cmd)
{
    dest->clear();

    Netpack::Builder packet(dest);
    cmd->pack(&packet);
    packet.prependWith16bitSize();
}

void clearVchAndBuildResyncPacket(vch *dest, Game* game)
{
    dest->clear();
    Netpack::Builder builder(dest);

    builder.packUint8_t(PACKET_RESYNC_CHAR);
    game->pack(&builder);

    builder.prependWith64bitSize();
}

void clearVchAndBuildFrameCmdsPacket(vch *dest, FrameEventsPacket fep)
{
    dest->clear();
    Netpack::Builder builder(dest);

    fep.pack(&builder);

    builder.prependWith64bitSize();
}
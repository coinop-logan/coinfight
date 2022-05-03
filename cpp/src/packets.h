#include <boost/shared_ptr.hpp>
#include "cmds.h"
#include "netpack.h"
#include "stdint.h"
#include "events.h"

#ifndef PACKETS_H
#define PACKETS_H

using namespace std;

class Packet
{
public:
    virtual uint8_t typechar();

    virtual void pack(Netpack::Builder*);

    void packPacketBasics(Netpack::Builder*);

    Packet();
    Packet(Netpack::Consumer*);
};

// note that a resync packet is so simple that it's just made directly in server and client

struct FrameEventsPacket : public Packet
{
    uint8_t typechar();

    uint64_t frame;
    vector<boost::shared_ptr<AuthdCmd>> authdCmds;
    vector<boost::shared_ptr<Event>> events;

    void pack(Netpack::Builder*);

    FrameEventsPacket(uint64_t frame, vector<boost::shared_ptr<AuthdCmd>> authdCmds, vector<boost::shared_ptr<Event>> events);
    FrameEventsPacket(Netpack::Consumer*);
};

#endif // PACKETS_H
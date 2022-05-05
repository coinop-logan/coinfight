#include <boost/shared_ptr.hpp>
#include "cmds.h"
#include "netpack.h"
#include "stdint.h"
#include "events.h"

#ifndef PACKETS_H
#define PACKETS_H

using namespace std;

using vch = vector<unsigned char>;

class Packet
{
public:
    virtual uint8_t typechar();

    virtual void pack(Netpack::Builder*);

    void packPacketBasics(Netpack::Builder*);

    Packet();
    Packet(Netpack::Consumer*);
};

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

void clearVchAndBuildCmdPacket(vch *dest, boost::shared_ptr<Cmd> cmd);
void clearVchAndBuildResyncPacket(vch *dest, Game* game);
void clearVchAndBuildFrameCmdsPacket(vch *dest, FrameEventsPacket fep);

#endif // PACKETS_H
#include <boost/shared_ptr.hpp>
#include "cmds.h"
#include "vchpack.h"
#include "stdint.h"

#ifndef PACKETS_H
#define PACKETS_H

using namespace std;

class Packet
{
public:
    virtual unsigned char typechar();

    virtual void pack(vch *dest);
    virtual void unpackAndMoveIter(vchIter *iter);

    void packPacket(vch *destVch);
    void unpackPacketAndMoveIter(vchIter *iter);

    Packet();
    Packet(vchIter *iter);
};

struct FrameCmdsPacket : public Packet
{
    unsigned char typechar();

    uint64_t frame;
    vector<boost::shared_ptr<Cmd>> cmds;

    void pack(vch *dest);
    void unpackAndMoveIter(vchIter *iter);

    FrameCmdsPacket(uint64_t frame, vector<boost::shared_ptr<Cmd>> cmds);
    FrameCmdsPacket(vchIter *iter);
};

#endif // PACKETS_H
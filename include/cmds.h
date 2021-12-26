#include <stdint.h>
#include <string>
#include "vchpack.h"
#include "myvectors.h"
#include "engine.h"
#include "config.h"

#ifndef CMDS_H
#define CMDS_H

using namespace std;

using vch = vector<unsigned char>;
using vchIter = vector<unsigned char>::iterator;
using EntityRef = uint16_t;

const unsigned char CMD_MOVE_CHAR = 'M';
const unsigned char CMD_PICKUP_CHAR = 'U';
const unsigned char CMD_PUTDOWN_CHAR = 'D';

struct Cmd
{
    vector<EntityRef> unitRefs;

    virtual string getTypename();
    virtual unsigned char getTypechar();
    virtual void pack(vch *dest);
    virtual void unpackAndMoveIter(vchIter *iter);

    vector<boost::shared_ptr<Unit>> getUnits(Game *game);
    void execute(Game *);
    virtual void executeOnUnit(boost::shared_ptr<Unit> unit);

    void packCmd(vch *dest);
    void unpackCmdAndMoveIter(vchIter *iter);

    Cmd(vector<EntityRef>);
    Cmd(vchIter *);
};

boost::shared_ptr<Cmd> unpackFullCmdAndMoveIter(vchIter *iter);

struct FrameCmdsPacket
{
    uint64_t frame;
    vector<boost::shared_ptr<Cmd>> cmds;

    void pack(vch *dest);
    void unpackAndMoveIter(vchIter *iter);

    FrameCmdsPacket(uint64_t frame, vector<boost::shared_ptr<Cmd>> cmds);
    FrameCmdsPacket(vchIter *iter);
};

struct MoveCmd : public Cmd
{
    vector2f pos;

    unsigned char getTypechar();
    string getTypename();
    void pack(vch *);
    void unpackAndMoveIter(vchIter *iter);

    void executeOnUnit(boost::shared_ptr<Unit>);

    MoveCmd(vector<EntityRef> unitRefs, vector2f pos);
    MoveCmd(vchIter *iter);
};

struct PickupCmd : public Cmd
{
    EntityRef goldRef;

    unsigned char getTypechar();
    string getTypename();
    void pack(vch *);
    void unpackAndMoveIter(vchIter *);

    void executeOnUnit(boost::shared_ptr<Unit>);

    PickupCmd(vector<EntityRef>, EntityRef);
    PickupCmd(vchIter *iter);
};

struct PutdownCmd : public Cmd
{
    Target target;

    unsigned char getTypechar();
    string getTypename();
    void pack(vch *);
    void unpackAndMoveIter(vchIter *);

    void executeOnUnit(boost::shared_ptr<Unit>);

    PutdownCmd(vector<EntityRef>, Target);
    PutdownCmd(vchIter *iter);
};


#endif // CMDS_H
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

const unsigned char CMD_MOVE_CHAR = 0;
const unsigned char CMD_PICKUP_CHAR = 1;
const unsigned char CMD_PUTDOWN_CHAR = 2;
const unsigned char CMD_SENDGOLDTHROUGHGATEWAY_CHAR = 3;
const unsigned char CMD_PUSHGOLDTHROUGHGATEWAY_CHAR = 4;

struct Cmd
{
    vector<EntityRef> unitRefs;

    virtual string getTypename();
    virtual unsigned char getTypechar();
    virtual void pack(vch *dest);
    virtual void unpackAndMoveIter(vchIter *iter);

    vector<boost::shared_ptr<Unit>> getUnits(Game *game);
    void executeIfOwnedBy(Game *, string userAddress);
    virtual void executeOnUnit(boost::shared_ptr<Unit> unit);

    void packCmd(vch *dest);
    void unpackCmdAndMoveIter(vchIter *iter);

    Cmd(vector<EntityRef>);
    Cmd(vchIter *);
};

struct AuthdCmd
{
    boost::shared_ptr<Cmd> cmd;
    string playerAddress;
    void execute(Game *);
    AuthdCmd(boost::shared_ptr<Cmd> cmd, string playerAddress);
};

boost::shared_ptr<Cmd> unpackFullCmdAndMoveIter(vchIter *iter);

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

struct SendGoldThroughGatewayCmd : public Cmd
{
    EntityRef gatewayRef;

    unsigned char getTypechar();
    string getTypename();
    void pack(vch *);
    void unpackAndMoveIter(vchIter *);

    void executeOnUnit(boost::shared_ptr<Unit>);

    SendGoldThroughGatewayCmd(vector<EntityRef>, EntityRef);
    SendGoldThroughGatewayCmd(vchIter *iter);
};

struct PushGoldThroughGatewayCmd : public Cmd
{
    EntityRef gatewayRef;

    unsigned char getTypechar();
    string getTypename();
    void pack(vch *);
    void unpackAndMoveIter(vchIter *);

    void executeOnUnit(boost::shared_ptr<Unit>);

    PushGoldThroughGatewayCmd(vector<EntityRef>, EntityRef);
    PushGoldThroughGatewayCmd(vchIter *iter);
};

#endif // CMDS_H
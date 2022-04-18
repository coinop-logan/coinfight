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

const unsigned char CMD_MOVE_CHAR = 0;
const unsigned char CMD_PICKUP_CHAR = 1;
const unsigned char CMD_PUTDOWN_CHAR = 2;
const unsigned char CMD_GATEWAYBUILD_CHAR = 3;
const unsigned char CMD_WITHDRAW_CHAR = 4;
const unsigned char CMD_ATTACK_CHAR = 5;
const unsigned char CMD_PRIMEBUILD_CHAR = 6;
const unsigned char CMD_RESUMEBUILDING_CHAR = 7;
const unsigned char CMD_SPAWNBEACON_CHAR = 8;
const unsigned char CMD_SCUTTLE_CHAR = 9;

struct Cmd
{
    virtual string getTypename();
    virtual unsigned char getTypechar();
    virtual void pack(vch *dest);
    virtual void unpackAndMoveIter(vchIter *iter);

    void packCmd(vch *dest);
    void unpackCmdAndMoveIter(vchIter *iter);

    Cmd();
    Cmd(vchIter *);
};

struct AuthdCmd
{
    boost::shared_ptr<Cmd> cmd;
    string playerAddress;
    AuthdCmd(boost::shared_ptr<Cmd> cmd, string playerAddress);
};

struct WithdrawCmd : public Cmd
{
    coinsInt amount;

    unsigned char getTypechar();
    string getTypename();
    void pack(vch *dest);
    void unpackAndMoveIter(vchIter *iter);

    WithdrawCmd(coinsInt amount);
    WithdrawCmd(vchIter *iter);
};

struct SpawnBeaconCmd : public Cmd
{
    vector2f pos;

    unsigned char getTypechar();
    string getTypename();
    void pack(vch *dest);
    void unpackAndMoveIter(vchIter *iter);

    void executeAsPlayer(Game* game, string playerAddress);

    SpawnBeaconCmd(vector2f pos);
    SpawnBeaconCmd(vchIter *iter);
};

boost::shared_ptr<Cmd> unpackFullCmdAndMoveIter(vchIter *iter);

struct UnitCmd : public Cmd
{
    vector<EntityRef> unitRefs;
    vector<boost::shared_ptr<Unit>> getUnits(Game *game);

    void executeAsPlayer(Game *, string userAddress);
    virtual void executeOnUnit(boost::shared_ptr<Unit> unit);

    void packUnitCmd(vch *dest);
    void unpackUnitCmdAndMoveIter(vchIter *iter);

    UnitCmd(vector<EntityRef> entityRefs);
    UnitCmd(vchIter *iter);
};

struct MoveCmd : public UnitCmd
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

struct PickupCmd : public UnitCmd
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

struct PutdownCmd : public UnitCmd
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

struct GatewayBuildCmd : public UnitCmd
{
    unsigned char buildTypechar;

    unsigned char getTypechar();
    string getTypename();
    void pack(vch *);
    void unpackAndMoveIter(vchIter *);

    void executeOnUnit(boost::shared_ptr<Unit>);

    GatewayBuildCmd(vector<EntityRef>, unsigned char buildTypechar);
    GatewayBuildCmd(vchIter *iter);
};

struct PrimeBuildCmd : public UnitCmd
{
    unsigned char buildTypechar;
    vector2f buildPos;

    unsigned char getTypechar();
    string getTypename();
    void pack(vch *);
    void unpackAndMoveIter(vchIter *);

    void executeOnUnit(boost::shared_ptr<Unit>);

    PrimeBuildCmd(vector<EntityRef>, unsigned char buildTypechar, vector2f buildPos);
    PrimeBuildCmd(vchIter *iter);
};

struct AttackCmd : public UnitCmd
{
    EntityRef targetUnit;

    unsigned char getTypechar();
    string getTypename();
    void pack(vch *);
    void unpackAndMoveIter(vchIter *);

    void executeOnUnit(boost::shared_ptr<Unit>);

    AttackCmd(vector<EntityRef>, EntityRef);
    AttackCmd(vchIter *iter);
};

struct ResumeBuildingCmd : public UnitCmd
{
    EntityRef targetUnit;

    unsigned char getTypechar();
    string getTypename();
    void pack(vch *);
    void unpackAndMoveIter(vchIter *);

    void executeOnUnit(boost::shared_ptr<Unit>);

    ResumeBuildingCmd(vector<EntityRef>, EntityRef);
    ResumeBuildingCmd(vchIter *iter);
};

struct ScuttleCmd : public UnitCmd
{
    EntityRef targetUnit;

    unsigned char getTypechar();
    string getTypename();
    void pack(vch *);
    void unpackAndMoveIter(vchIter *);

    void executeOnUnit(boost::shared_ptr<Unit>);

    ScuttleCmd(vector<EntityRef>, EntityRef);
    ScuttleCmd(vchIter *iter);
};

#endif // CMDS_H

#include <stdint.h>
#include <string>
#include "netpack.h"
#include "myvectors.h"
#include "engine.h"
#include "config.h"

#ifndef CMDS_H
#define CMDS_H

using namespace std;

const uint8_t CMD_SPAWNBEACON_CHAR = 1;
const uint8_t CMD_WITHDRAW_CHAR = 2;
const uint8_t CMD_MOVE_CHAR = 3;
const uint8_t CMD_ATTACKABSORB_CHAR = 4;
const uint8_t CMD_STOP_CHAR = 5;
const uint8_t CMD_FETCH_CHAR = 6;
const uint8_t CMD_DEPOSIT_CHAR = 7;
const uint8_t CMD_GATEWAYBUILD_CHAR = 8;
const uint8_t CMD_PRIMEBUILD_CHAR = 9;
const uint8_t CMD_GIFT_CHAR = 10;

struct Cmd
{
    virtual string getTypename();
    virtual uint8_t getTypechar();
    virtual void pack(Netpack::Builder* to);

    void packCmdBasics(Netpack::Builder* to);

    Cmd();
    Cmd(Netpack::Consumer*);
};

boost::shared_ptr<Cmd> consumeCmd(Netpack::Consumer* from);

struct AuthdCmd
{
    boost::shared_ptr<Cmd> cmd;
    Address playerAddress;
    AuthdCmd(boost::shared_ptr<Cmd> cmd, Address playerAddress);
};

struct WithdrawCmd : public Cmd
{
    coinsInt amount;

    uint8_t getTypechar();
    string getTypename();
    void pack(Netpack::Builder* to);

    WithdrawCmd(coinsInt amount);
    WithdrawCmd(Netpack::Consumer* from);
};

struct SpawnBeaconCmd : public Cmd
{
    vector2fp pos;

    uint8_t getTypechar();
    string getTypename();
    void pack(Netpack::Builder* to);

    void executeAsPlayer(Game* game, Address playerAddress);

    SpawnBeaconCmd(vector2fp pos);
    SpawnBeaconCmd(Netpack::Consumer* from);
};

struct UnitCmd : public Cmd
{
    vector<EntityRef> unitRefs;
    vector<boost::shared_ptr<Unit>> getUnits(Game *game);

    void executeAsPlayer(Game *, Address userAddress);
    virtual void prepForUnitExecution(Game *, uint8_t);
    virtual void executeOnUnit(boost::shared_ptr<Unit> unit);

    void packUnitCmdBasics(Netpack::Builder* to);

    UnitCmd(vector<EntityRef> entityRefs);
    UnitCmd(Netpack::Consumer* from);
};

struct MoveCmd : public UnitCmd
{
    vector2fp pos;

    uint8_t getTypechar();
    string getTypename();
    void pack(Netpack::Builder* to);

    void executeOnUnit(boost::shared_ptr<Unit>);

    MoveCmd(vector<EntityRef> unitRefs, vector2fp pos);
    MoveCmd(Netpack::Consumer* from);
};

struct StopCmd : public UnitCmd
{
    uint8_t getTypechar();
    string getTypename();
    void pack(Netpack::Builder* to);

    void executeOnUnit(boost::shared_ptr<Unit>);

    StopCmd(vector<EntityRef> unitRefs);
    StopCmd(Netpack::Consumer* from);
};

struct FetchCmd : public UnitCmd
{
    Target target;

    uint8_t getTypechar();
    string getTypename();
    void pack(Netpack::Builder* to);

    void executeOnUnit(boost::shared_ptr<Unit>);

    FetchCmd(vector<EntityRef>, Target);
    FetchCmd(Netpack::Consumer* from);
};

struct DepositCmd : public UnitCmd
{
    Target target;

    // Not meant for network transport
    boost::shared_ptr<Entity> entityToDepositTo;

    uint8_t getTypechar();
    string getTypename();
    void pack(Netpack::Builder* to);

    void prepForUnitExecution(Game*, uint8_t ownerId);
    void executeOnUnit(boost::shared_ptr<Unit>);

    DepositCmd(vector<EntityRef>, Target);
    DepositCmd(Netpack::Consumer* from);
};

struct GatewayBuildCmd : public UnitCmd
{
    uint8_t buildTypechar;

    uint8_t getTypechar();
    string getTypename();
    void pack(Netpack::Builder* to);

    void executeOnUnit(boost::shared_ptr<Unit>);

    GatewayBuildCmd(vector<EntityRef>, uint8_t buildTypechar);
    GatewayBuildCmd(Netpack::Consumer* from);
};

struct PrimeBuildCmd : public UnitCmd
{
    uint8_t buildTypechar;
    vector2fp buildPos;

    // Not meant for network transport
    boost::shared_ptr<Unit> unitToBuild;

    uint8_t getTypechar();
    string getTypename();
    void pack(Netpack::Builder* to);

    void prepForUnitExecution(Game*, uint8_t ownerId);
    void executeOnUnit(boost::shared_ptr<Unit>);

    PrimeBuildCmd(vector<EntityRef>, uint8_t buildTypechar, vector2fp buildPos);
    PrimeBuildCmd(Netpack::Consumer* from);
};

struct AttackAbsorbCmd : public UnitCmd
{
    Target target;

    uint8_t getTypechar();
    string getTypename();
    void pack(Netpack::Builder* to);

    void executeOnUnit(boost::shared_ptr<Unit>);

    AttackAbsorbCmd(vector<EntityRef>, Target);
    AttackAbsorbCmd(Netpack::Consumer* from);
};

struct GiftCmd : public UnitCmd
{
    uint8_t newOwnerId;

    uint8_t getTypechar();
    string getTypename();
    void pack(Netpack::Builder* to);

    void executeOnUnit(boost::shared_ptr<Unit>);

    GiftCmd(vector<EntityRef>, uint8_t);
    GiftCmd(Netpack::Consumer* from);
};

#endif // CMDS_H

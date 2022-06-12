#ifndef UNIT_INTERFACE_CMDS_H
#define UNIT_INTERFACE_CMDS_H

#include <SFML/Graphics.hpp>
#include <boost/shared_ptr.hpp>
#include "cmds.h"
#include "input.h"

using namespace std;

struct UI; // defined in input.h

struct InterfaceCmd
{
    virtual sf::Keyboard::Key getKey();
    virtual vector<boost::shared_ptr<Cmd>> execute(UI *ui);
};

struct SpawnBeaconInterfaceCmd : public InterfaceCmd
{
    sf::Keyboard::Key getKey();
    vector<boost::shared_ptr<Cmd>> execute(UI *ui);
};

struct UnitInterfaceCmd : public InterfaceCmd
{
    virtual sf::Keyboard::Key getKey();
    virtual vector<boost::shared_ptr<Cmd>> execute(UI *ui);
    virtual bool isUnitEligible(boost::shared_ptr<Unit> unit);
};

struct DepositInterfaceCmd : public UnitInterfaceCmd
{
    sf::Keyboard::Key getKey();
    vector<boost::shared_ptr<Cmd>> execute(UI *ui);
    bool isUnitEligible(boost::shared_ptr<Unit> unit);
};
struct GatewayBuildPrimeInterfaceCmd : public UnitInterfaceCmd
{
    sf::Keyboard::Key getKey();
    vector<boost::shared_ptr<Cmd>> execute(UI *ui);
    bool isUnitEligible(boost::shared_ptr<Unit> unit);
};
struct GatewayBuildFighterInterfaceCmd : public UnitInterfaceCmd
{
    sf::Keyboard::Key getKey();
    vector<boost::shared_ptr<Cmd>> execute(UI *ui);
    bool isUnitEligible(boost::shared_ptr<Unit> unit);
};
struct PrimeBuildGatewayInterfaceCmd : public UnitInterfaceCmd
{
    sf::Keyboard::Key getKey();
    vector<boost::shared_ptr<Cmd>> execute(UI *ui);
    bool isUnitEligible(boost::shared_ptr<Unit> unit);
};
struct PrimeBuildTurretInterfaceCmd : public UnitInterfaceCmd
{
    sf::Keyboard::Key getKey();
    vector<boost::shared_ptr<Cmd>> execute(UI *ui);
    bool isUnitEligible(boost::shared_ptr<Unit> unit);
};
struct ScuttleInterfaceCmd : public UnitInterfaceCmd
{
    sf::Keyboard::Key getKey();
    vector<boost::shared_ptr<Cmd>> execute(UI *ui);
    bool isUnitEligible(boost::shared_ptr<Unit> unit);
};
struct AttackGatherInterfaceCmd : public UnitInterfaceCmd
{
    sf::Keyboard::Key getKey();
    vector<boost::shared_ptr<Cmd>> execute(UI *ui);
    bool isUnitEligible(boost::shared_ptr<Unit> unit);
};

#endif // UNIT_INTERFACE_CMDS_H
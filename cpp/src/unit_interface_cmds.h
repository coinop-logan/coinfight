#ifndef UNIT_INTERFACE_CMDS_H
#define UNIT_INTERFACE_CMDS_H

#include <SFML/Graphics.hpp>
#include <boost/shared_ptr.hpp>
#include "cmds.h"

using namespace std;

struct InterfaceCmd
{
    bool eligible, active;
    int visualFlashStart;
    InterfaceCmd()
        : eligible(false), active(false)
        {}
    virtual sf::Keyboard::Key getKey();
    virtual const tuple<sf::Keyboard::Key, char, vector<string>, optional<coinsInt>> getHotkeyInfo() const;
};

struct SpawnBeaconInterfaceCmd : public InterfaceCmd
{
    sf::Keyboard::Key getKey();
    const tuple<sf::Keyboard::Key, char, vector<string>, optional<coinsInt>> getHotkeyInfo() const;
};

struct UnitInterfaceCmd : public InterfaceCmd
{
    virtual sf::Keyboard::Key getKey();
    virtual bool isUnitEligible(boost::shared_ptr<Unit> unit);
    virtual const tuple<sf::Keyboard::Key, char, vector<string>, optional<coinsInt>> getHotkeyInfo() const;
};

struct AttackAbsorbInterfaceCmd : public UnitInterfaceCmd
{
    sf::Keyboard::Key getKey();
    bool isUnitEligible(boost::shared_ptr<Unit> unit);
    const tuple<sf::Keyboard::Key, char, vector<string>, optional<coinsInt>> getHotkeyInfo() const;
};
struct StopInterfaceCmd : public UnitInterfaceCmd
{
    sf::Keyboard::Key getKey();
    bool isUnitEligible(boost::shared_ptr<Unit> unit);
    const tuple<sf::Keyboard::Key, char, vector<string>, optional<coinsInt>> getHotkeyInfo() const;
};
struct DepositInterfaceCmd : public UnitInterfaceCmd
{
    sf::Keyboard::Key getKey();
    bool isUnitEligible(boost::shared_ptr<Unit> unit);
    const tuple<sf::Keyboard::Key, char, vector<string>, optional<coinsInt>> getHotkeyInfo() const;
};
struct FetchInterfaceCmd : public UnitInterfaceCmd
{
    sf::Keyboard::Key getKey();
    bool isUnitEligible(boost::shared_ptr<Unit> unit);
    const tuple<sf::Keyboard::Key, char, vector<string>, optional<coinsInt>> getHotkeyInfo() const;
};
struct GatewayBuildPrimeInterfaceCmd : public UnitInterfaceCmd
{
    sf::Keyboard::Key getKey();
    bool isUnitEligible(boost::shared_ptr<Unit> unit);
    const tuple<sf::Keyboard::Key, char, vector<string>, optional<coinsInt>> getHotkeyInfo() const;
};
struct GatewayBuildFighterInterfaceCmd : public UnitInterfaceCmd
{
    sf::Keyboard::Key getKey();
    bool isUnitEligible(boost::shared_ptr<Unit> unit);
    const tuple<sf::Keyboard::Key, char, vector<string>, optional<coinsInt>> getHotkeyInfo() const;
};
struct PrimeBuildGatewayInterfaceCmd : public UnitInterfaceCmd
{
    sf::Keyboard::Key getKey();
    bool isUnitEligible(boost::shared_ptr<Unit> unit);
    const tuple<sf::Keyboard::Key, char, vector<string>, optional<coinsInt>> getHotkeyInfo() const;
};
struct PrimeBuildTurretInterfaceCmd : public UnitInterfaceCmd
{
    sf::Keyboard::Key getKey();
    bool isUnitEligible(boost::shared_ptr<Unit> unit);
    const tuple<sf::Keyboard::Key, char, vector<string>, optional<coinsInt>> getHotkeyInfo() const;
};

#endif // UNIT_INTERFACE_CMDS_H
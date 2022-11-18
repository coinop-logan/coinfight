#include "unit_interface_cmds.h"

sf::Keyboard::Key InterfaceCmd::getKey()
{
    throw runtime_error("getKey() has not been defined for this uxCmd.");
}
const tuple<sf::Keyboard::Key, char, vector<string>, optional<coinsInt>> InterfaceCmd::getHotkeyInfo() const
{
    throw runtime_error("getHotkeyInfo() has not been defined for this uxCmd.");
}

sf::Keyboard::Key SpawnBeaconInterfaceCmd::getKey()
{
    return sf::Keyboard::B;
}



const tuple<sf::Keyboard::Key, char, vector<string>, optional<coinsInt>> SpawnBeaconInterfaceCmd::getHotkeyInfo() const
{
    return {sf::Keyboard::B, 'B', {"Spawn", "Beacon"}, {GATEWAY_COST}};
}
const tuple<sf::Keyboard::Key, char, vector<string>, optional<coinsInt>> GatewayBuildPrimeInterfaceCmd::getHotkeyInfo() const
{
    return {sf::Keyboard::Q, 'Q', {"Build", "Prime"}, {PRIME_COST}};
}
const tuple<sf::Keyboard::Key, char, vector<string>, optional<coinsInt>> GatewayBuildFighterInterfaceCmd::getHotkeyInfo() const
{
    return {sf::Keyboard::W, 'W', {"Build", "Fighter"}, {FIGHTER_COST}};
}
const tuple<sf::Keyboard::Key, char, vector<string>, optional<coinsInt>> PrimeBuildGatewayInterfaceCmd::getHotkeyInfo() const
{
    return {sf::Keyboard::E, 'E', {"Build", "Gateway"}, {GATEWAY_COST}};
}
const tuple<sf::Keyboard::Key, char, vector<string>, optional<coinsInt>> PrimeBuildTurretInterfaceCmd::getHotkeyInfo() const
{
    return {sf::Keyboard::R, 'R', {"Build", "Turret"}, {TURRET_COST}};
}
const tuple<sf::Keyboard::Key, char, vector<string>, optional<coinsInt>> AttackAbsorbInterfaceCmd::getHotkeyInfo() const
{
    return {sf::Keyboard::A, 'A', {"Attack/", "Absorb"}, {}};
}
const tuple<sf::Keyboard::Key, char, vector<string>, optional<coinsInt>> StopInterfaceCmd::getHotkeyInfo() const
{
    return {sf::Keyboard::S, 'S', {"Stop"}, {}};
}
const tuple<sf::Keyboard::Key, char, vector<string>, optional<coinsInt>> DepositInterfaceCmd::getHotkeyInfo() const
{
    return {sf::Keyboard::D, 'D', {"Deposit"}, {}};
}
const tuple<sf::Keyboard::Key, char, vector<string>, optional<coinsInt>> FetchInterfaceCmd::getHotkeyInfo() const
{
    return {sf::Keyboard::F, 'F', {"Fetch"}, {}};
}



sf::Keyboard::Key UnitInterfaceCmd::getKey()
{
    throw runtime_error("getKey() has not been defined for this uxCmd.");
}

bool UnitInterfaceCmd::isUnitEligible(boost::shared_ptr<Unit> unit)
{
    throw runtime_error("isUnitEligible() has not been defined for this uxCmd.");
}
const tuple<sf::Keyboard::Key, char, vector<string>, optional<coinsInt>> UnitInterfaceCmd::getHotkeyInfo() const
{
    throw runtime_error("getHotkeyInfo() has not been defined for this uxCmd.");
}




sf::Keyboard::Key AttackAbsorbInterfaceCmd::getKey()
{
    return sf::Keyboard::A;
}
bool AttackAbsorbInterfaceCmd::isUnitEligible(boost::shared_ptr<Unit> unit)
{
    return (unit->typechar() == FIGHTER_TYPECHAR || unit->typechar() == PRIME_TYPECHAR || unit->typechar() == GATEWAY_TYPECHAR);
}




sf::Keyboard::Key StopInterfaceCmd::getKey()
{
    return sf::Keyboard::S;
}
bool StopInterfaceCmd::isUnitEligible(boost::shared_ptr<Unit> unit)
{
    return (unit->typechar() == PRIME_TYPECHAR || unit->typechar() == GATEWAY_TYPECHAR);
}




sf::Keyboard::Key DepositInterfaceCmd::getKey()
{
    return sf::Keyboard::D;
}
bool DepositInterfaceCmd::isUnitEligible(boost::shared_ptr<Unit> unit)
{
    return (unit->typechar() == PRIME_TYPECHAR || unit->typechar() == GATEWAY_TYPECHAR);
}




sf::Keyboard::Key FetchInterfaceCmd::getKey()
{
    return sf::Keyboard::F;
}
bool FetchInterfaceCmd::isUnitEligible(boost::shared_ptr<Unit> unit)
{
    return (unit->typechar() == PRIME_TYPECHAR || unit->typechar() == GATEWAY_TYPECHAR);
}




sf::Keyboard::Key GatewayBuildPrimeInterfaceCmd::getKey()
{
    return sf::Keyboard::Q;
}
bool GatewayBuildPrimeInterfaceCmd::isUnitEligible(boost::shared_ptr<Unit> unit)
{
    return (unit->typechar() == GATEWAY_TYPECHAR);
}




sf::Keyboard::Key GatewayBuildFighterInterfaceCmd::getKey()
{
    return sf::Keyboard::W;
}
bool GatewayBuildFighterInterfaceCmd::isUnitEligible(boost::shared_ptr<Unit> unit)
{
    return (unit->typechar() == GATEWAY_TYPECHAR);
}




sf::Keyboard::Key PrimeBuildGatewayInterfaceCmd::getKey()
{
    return sf::Keyboard::E;
}
bool PrimeBuildGatewayInterfaceCmd::isUnitEligible(boost::shared_ptr<Unit> unit)
{
    return (unit->typechar() == PRIME_TYPECHAR);
}




sf::Keyboard::Key PrimeBuildTurretInterfaceCmd::getKey()
{
    return sf::Keyboard::R;
}
bool PrimeBuildTurretInterfaceCmd::isUnitEligible(boost::shared_ptr<Unit> unit)
{
    return (unit->typechar() == PRIME_TYPECHAR);
}
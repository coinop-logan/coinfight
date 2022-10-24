#include "unit_interface_cmds.h"

vector<boost::shared_ptr<Cmd>> noCmds;

sf::Keyboard::Key InterfaceCmd::getKey()
{
    throw runtime_error("getKey() has not been defined for this uxCmd.");
}
vector<boost::shared_ptr<Cmd>> InterfaceCmd::execute(UI *ui)
{
    throw runtime_error("execute() has not been defined for this uxCmd.");
}

sf::Keyboard::Key SpawnBeaconInterfaceCmd::getKey()
{
    return sf::Keyboard::B;
}
vector<boost::shared_ptr<Cmd>> SpawnBeaconInterfaceCmd::execute(UI *ui)
{
    ui->cmdState = UI::SpawnBeacon;
    return noCmds;
}




sf::Keyboard::Key UnitInterfaceCmd::getKey()
{
    throw runtime_error("getKey() has not been defined for this uxCmd.");
}
vector<boost::shared_ptr<Cmd>> UnitInterfaceCmd::execute(UI *ui)
{
    throw runtime_error("execute() has not been defined for this uxCmd.");
}

bool UnitInterfaceCmd::isUnitEligible(boost::shared_ptr<Unit> unit)
{
    throw runtime_error("isUnitEligible() has not been defined for this uxCmd.");
}




sf::Keyboard::Key StopInterfaceCmd::getKey()
{
    return sf::Keyboard::S;
}
vector<boost::shared_ptr<Cmd>> StopInterfaceCmd::execute(UI *ui)
{
    if (ui->selectedUnits.size() > 0)
    {
        
        return {boost::shared_ptr<StopCmd>(new StopCmd(entityPtrsToRefsOrThrow(ui->selectedUnits)))};
    }
    else
    {
        return noCmds;
    }
}
bool StopInterfaceCmd::isUnitEligible(boost::shared_ptr<Unit> unit)
{
    return (unit->typechar() == PRIME_TYPECHAR || unit->typechar() == GATEWAY_TYPECHAR);
}




sf::Keyboard::Key DepositInterfaceCmd::getKey()
{
    return sf::Keyboard::D;
}
vector<boost::shared_ptr<Cmd>> DepositInterfaceCmd::execute(UI *ui)
{
    ui->cmdState = UI::Deposit;
    return noCmds;
}
bool DepositInterfaceCmd::isUnitEligible(boost::shared_ptr<Unit> unit)
{
    return (unit->typechar() == PRIME_TYPECHAR || unit->typechar() == GATEWAY_TYPECHAR);
}




sf::Keyboard::Key GatewayBuildPrimeInterfaceCmd::getKey()
{
    return sf::Keyboard::Q;
}
vector<boost::shared_ptr<Cmd>> GatewayBuildPrimeInterfaceCmd::execute(UI *ui)
{
    if (auto cmd = makeGatewayBuildCmd(ui->selectedUnits, PRIME_TYPECHAR))
        return {cmd};
    else
        return noCmds;
}
bool GatewayBuildPrimeInterfaceCmd::isUnitEligible(boost::shared_ptr<Unit> unit)
{
    return (unit->typechar() == GATEWAY_TYPECHAR);
}




sf::Keyboard::Key GatewayBuildFighterInterfaceCmd::getKey()
{
    return sf::Keyboard::W;
}
vector<boost::shared_ptr<Cmd>> GatewayBuildFighterInterfaceCmd::execute(UI *ui)
{
    if (auto cmd = makeGatewayBuildCmd(ui->selectedUnits, FIGHTER_TYPECHAR))
        return {cmd};
    else
        return noCmds;
}
bool GatewayBuildFighterInterfaceCmd::isUnitEligible(boost::shared_ptr<Unit> unit)
{
    return (unit->typechar() == GATEWAY_TYPECHAR);
}




sf::Keyboard::Key PrimeBuildGatewayInterfaceCmd::getKey()
{
    return sf::Keyboard::E;
}
vector<boost::shared_ptr<Cmd>> PrimeBuildGatewayInterfaceCmd::execute(UI *ui)
{
    ui->cmdState = UI::Build;
    ui->ghostBuilding = boost::shared_ptr<Building>(new Gateway(-1, vector2fp::zero));

    return noCmds;
}
bool PrimeBuildGatewayInterfaceCmd::isUnitEligible(boost::shared_ptr<Unit> unit)
{
    return (unit->typechar() == PRIME_TYPECHAR);
}




sf::Keyboard::Key PrimeBuildTurretInterfaceCmd::getKey()
{
    return sf::Keyboard::R;
}
vector<boost::shared_ptr<Cmd>> PrimeBuildTurretInterfaceCmd::execute(UI *ui)
{
    ui->cmdState = UI::Build;
    ui->ghostBuilding = boost::shared_ptr<Building>(new Turret(-1, vector2fp::zero));

    return noCmds;
}
bool PrimeBuildTurretInterfaceCmd::isUnitEligible(boost::shared_ptr<Unit> unit)
{
    return (unit->typechar() == PRIME_TYPECHAR);
}




sf::Keyboard::Key ScuttleInterfaceCmd::getKey()
{
    return sf::Keyboard::F;
}
vector<boost::shared_ptr<Cmd>> ScuttleInterfaceCmd::execute(UI *ui)
{
    ui->cmdState = UI::Scuttle;

    return noCmds;
}
bool ScuttleInterfaceCmd::isUnitEligible(boost::shared_ptr<Unit> unit)
{
    return (unit->typechar() == PRIME_TYPECHAR || unit->typechar() == GATEWAY_TYPECHAR);
}




sf::Keyboard::Key AttackGatherInterfaceCmd::getKey()
{
    return sf::Keyboard::A;
}
vector<boost::shared_ptr<Cmd>> AttackGatherInterfaceCmd::execute(UI *ui)
{
    ui->cmdState = UI::AttackGather;

    return noCmds;
}
bool AttackGatherInterfaceCmd::isUnitEligible(boost::shared_ptr<Unit> unit)
{
    return (unit->typechar() == FIGHTER_TYPECHAR || unit->typechar() == PRIME_TYPECHAR);
}
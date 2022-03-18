#include "unit_interface_cmds.h"

vector<boost::shared_ptr<Cmd>> noCmds;

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
    return (unit->typechar() == PRIME_TYPECHAR);
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
    ui->ghostBuilding = boost::shared_ptr<Building>(new Gateway(NULL, 0, -1, vector2f(0,0)));

    return noCmds;
}
bool PrimeBuildGatewayInterfaceCmd::isUnitEligible(boost::shared_ptr<Unit> unit)
{
    return (unit->typechar() == PRIME_TYPECHAR);
}

// case sf::Keyboard::D:
//     if (filterForType<Prime, Entity>(ui->selectedEntities).size() > 0)
//     {
//         ui->cmdState = UI::Deposit;
//     }
//     break;
// case sf::Keyboard::Q:
//     if (auto cmd = makeGatewayBuildCmd(ui->selectedEntities, PRIME_TYPECHAR))
//         cmdsToSend.push_back(cmd);
//     break;
// case sf::Keyboard::W:
//     if (auto cmd = makeGatewayBuildCmd(ui->selectedEntities, FIGHTER_TYPECHAR))
//         cmdsToSend.push_back(cmd);
//     break;
// case sf::Keyboard::E:
//     if (filterForType<Prime, Entity>(ui->selectedEntities).size() > 0)
//     {
//         ui->cmdState = UI::Build;
//         ui->ghostBuilding = boost::shared_ptr<Building>(new Gateway(game, 0, playerId, vector2f(0,0)));
//     }
//     break;
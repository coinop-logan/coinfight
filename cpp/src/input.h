#ifndef INPUT_H
#define INPUT_H

#include "myvectors.h"
#include "common.h"
#include "cmds.h"
#include "unit_interface_cmds.h"

struct UnitInterfaceCmd; // defined in unit_interface_cmds.h

struct CameraState
{
    vector2i gamePos;
};

struct UnitInterfaceCmdWithState
{
    bool eligible;
    boost::shared_ptr<UnitInterfaceCmd> interfaceCmd;
    UnitInterfaceCmdWithState(boost::shared_ptr<UnitInterfaceCmd> interfaceCmd)
        : eligible(false), interfaceCmd(interfaceCmd) {}
};

struct UI
{
    UI();
    boost::shared_ptr<Entity> mouseoverEntity;
    boost::shared_ptr<Building> ghostBuilding;
    enum CmdState {
        Default,
        Deposit,
        Build
    } cmdState;
    vector2i lastMousePos;
    vector<boost::shared_ptr<Unit>> selectedUnits;
    CameraState camera;
    vector<UnitInterfaceCmdWithState> interfaceCmdsWithState;
    int debugInt;
    void updateAvailableUnitInterfaceCmds();
    vector<boost::shared_ptr<Cmd>> handlePossibleUnitInterfaceCmd(sf::Keyboard::Key);
};
vector2f screenPosToGamePos(CameraState, vector2i);
vector2i gamePosToScreenPos(CameraState cameraState, vector2i gamePos);
Target getTargetAtScreenPos(const Game &, const CameraState &, vector2i);
boost::shared_ptr<Cmd> makeRightclickCmd(const Game &game, UI ui, Target target);
vector2i mouseButtonToVec(sf::Event::MouseButtonEvent mEvent);
vector<boost::shared_ptr<Cmd>> pollWindowEventsAndUpdateUI(Game *game, UI *ui, int playerId, sf::RenderWindow *window);

boost::shared_ptr<Cmd> makeGatewayBuildCmd(vector<boost::shared_ptr<Unit>> selectedUnits, unsigned char buildUnitTypechar);
boost::shared_ptr<Cmd> makePrimeBuildCmd(vector<boost::shared_ptr<Unit>> selectedUnits, unsigned char buildUnitTypechar, vector2f buildPos);

#endif // INPUT_H
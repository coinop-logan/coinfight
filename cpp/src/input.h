#include "myvectors.h"
#include "common.h"
#include "cmds.h"
#include "unit_interface_cmds.h"
#include "interface.h"

#ifndef INPUT_H
#define INPUT_H

const int SCREEN_EDGE_SCROLL_AMOUNT = 15;

class Tutorial; // define3d in tutorial.h

struct CameraState
{
    vector2i gamePos;
};

struct UI
{
    UI();
    sf::Clock lClickClock;
    boost::shared_ptr<Entity> mouseoverEntity;
    boost::shared_ptr<Building> ghostBuilding;
    enum CmdState {
        Default,
        SpawnBeacon,
        AttackAbsorb,
        Deposit,
        Fetch,
        Build
    } cmdState;
    bool minimapEnabled;
    vector2i lastMousePos;
    optional<vector2i> maybeSelectionBoxStart;
    vector<boost::shared_ptr<Unit>> selectedUnits;
    CameraState camera;
    vector<boost::shared_ptr<UnitInterfaceCmd>> unitInterfaceCmds;
    SpawnBeaconInterfaceCmd spawnBeaconInterfaceCmd;
    int countdownToQuitOrNeg1;
    bool quitNow;
    int escapeTextCountdownOrNeg1;
    int debugInt;
    bool cleanDrawEnabled;
    bool showTutorial;
    void updateAvailableUnitInterfaceCmds(bool spawnBeaconAvailable);
    void selectAllUnitsOfSimilarTypeOnScreen(Game*, boost::shared_ptr<Unit>);
    vector<boost::shared_ptr<Cmd>> handlePossibleUnitInterfaceCmd(sf::Keyboard::Key);
    bool selectionHasGateways();
    bool selectionWouldStaySegregated(uint8_t typechar); // checks if adding the unit type would mix Gateways/others
    void startEscapeToQuit();
    void cancelEscapeToQuit();
    void iterate();
    void removeDuplicatesFromSelection();
    void returnToDefaultState();
};
vector2fp screenPosToGamePos(CameraState, vector2i);
vector2i gamePosToScreenPos(CameraState cameraState, vector2fp gamePos);
Target getTargetAtScreenPos(Game *, const CameraState &, vector2i);
boost::shared_ptr<Cmd> makeRightclickCmd(const Game &game, UI ui, Target target);
vector2i mouseButtonToVec(sf::Event::MouseButtonEvent mEvent);
vector<boost::shared_ptr<Cmd>> pollWindowEventsAndUpdateUI(Game *game, UI *ui, optional<uint8_t> maybePlayerId, sf::RenderWindow *window, Tutorial* tutorial);

boost::shared_ptr<Cmd> makeGatewayBuildCmd(vector<boost::shared_ptr<Unit>> selectedUnits, uint8_t buildUnitTypechar);
boost::shared_ptr<Cmd> makePrimeBuildCmd(vector<boost::shared_ptr<Unit>> selectedUnits, uint8_t buildUnitTypechar, vector2fl buildPos, bool asap);

#endif // INPUT_H
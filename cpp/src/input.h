#include "myvectors.h"
#include "common.h"
#include "cmds.h"
#include "ui_elements.h"
#include "particles.h"

#ifndef INPUT_H
#define INPUT_H

const int SCREEN_EDGE_SCROLL_AMOUNT = 15;
const float ZOOM_AMOUNT = 1.2;

class Tutorial; // defined in tutorial.h

enum InGameMenuMsg {
    Resume,
    GiftUnits,
    Withdraw,
    Disconnect,
    WithdrawAndDisconnect
};

struct GameUI
{
    sf::Font* font;
    optional<MainMenu<InGameMenuMsg>> inGameMenu;
    optional<GiftUnitsWindow> giftUnitsWindow;
    sf::Clock lClickClock;
    boost::shared_ptr<Entity> mouseoverEntity;
    boost::shared_ptr<Building> ghostBuilding;
    vector<boost::shared_ptr<Unit>> selectedUnits;
    UnitInfoUXBox unitInfoBox;
    KeyButtonUXBox keyButtonBox;
    enum CmdState {
        Default,
        SpawnBeacon,
        AttackScuttle,
        Deposit,
        Fetch,
        Build
    } cmdState;
    bool minimapEnabled;
    vector2i lastMousePos;
    optional<vector2i> maybeSelectionBoxStart;
    sf::View cameraView;
    bool quitNow;
    int debugInt;
    bool cleanDrawEnabled;
    bool hideUX;
    bool showTutorial;
    bool online;
    bool displayAllRadii;
    ParticlesContainer particles;
    GameUI(sf::RenderWindow*, sf::Font* font, sf::Sprite* (*getSpriteForMsg)(KeyButtonMsg), sf::View view, bool online);
    void updateUnitCmds(bool spawnBeaconAvailable);
    void selectAllUnitsOfSimilarTypeOnScreen(sf::RenderWindow*, Game*, boost::shared_ptr<Unit>);
    bool selectionHasGateways();
    bool selectionWouldStaySegregated(uint8_t typechar); // checks if adding the unit type would mix Gateways/others
    void openInGameMenu(sf::RenderWindow*);
    void iterate();
    void removeDuplicatesFromSelection();
    void returnToDefaultState();
    tuple<bool, optional<boost::shared_ptr<Cmd>>> processEventForOverlay(sf::Event event, float uxViewZoom);
};
vector2fp screenPosToGamePos(sf::RenderWindow*, vector2i);
vector2i gamePosToScreenPos(sf::RenderWindow*, vector2fp);
Target getTargetAtScreenPos(sf::RenderWindow*, Game *, vector2i);
boost::shared_ptr<Cmd> makeRightclickCmd(const Game &game, GameUI ui, Target target);
// vector2fl mouseEventVec(sf::Event::MouseButtonEvent mEvent, sf::View view);
// vector2fl mouseEventVec(sf::Event::MouseMoveEvent mEvent, sf::View view);
vector2i mouseEventVec(sf::Event::MouseButtonEvent mEvent);
vector2i mouseEventVec(sf::Event::MouseMoveEvent mEvent);
vector<boost::shared_ptr<Cmd>> pollWindowEventsAndUpdateUI(Game *game, GameUI *ui, optional<uint8_t> maybePlayerId, sf::RenderWindow *window, Tutorial* tutorial, sf::View uxView);

boost::shared_ptr<Cmd> makeGatewayBuildCmd(vector<boost::shared_ptr<Unit>> selectedUnits, uint8_t buildUnitTypechar);
boost::shared_ptr<Cmd> makePrimeBuildCmd(vector<boost::shared_ptr<Unit>> selectedUnits, uint8_t buildUnitTypechar, vector2fl buildPos, bool asap);

#endif // INPUT_H

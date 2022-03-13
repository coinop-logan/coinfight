#include "myvectors.h"
#include "common.h"
#include "cmds.h"

#ifndef INPUT_H
#define INPUT_H

struct CameraState
{
    vector2i gamePos;
};

struct UI
{
    UI();
    boost::shared_ptr<Entity> mouseoverEntity;
    enum CmdState {
        Default,
        Deposit
    } cmdState;
    vector2i lastMousePos;
    vector<boost::shared_ptr<Entity>> selectedEntities;
    CameraState camera;
    int debugInt;
};
vector2f screenPosToGamePos(CameraState, vector2i);
vector2i gamePosToScreenPos(CameraState cameraState, vector2i gamePos);
Target getTargetAtScreenPos(const Game &, const CameraState &, vector2i);
boost::shared_ptr<Cmd> makeRightclickCmd(const Game &game, UI ui, Target target);
vector2i mouseButtonToVec(sf::Event::MouseButtonEvent mEvent);
vector<boost::shared_ptr<Cmd>> pollWindowEventsAndUpdateUI(const Game &game, UI *ui, int playerId, sf::RenderWindow *window);

#endif // INPUT_H
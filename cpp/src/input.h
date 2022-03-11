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
    // vector2f lastMouseMovePos;
    vector<boost::shared_ptr<Entity>> selectedEntities;
    CameraState camera;
};
vector2f screenPosToGamePos(CameraState, vector2i);
vector2i gamePosToScreenPos(CameraState cameraState, vector2i gamePos);
Target getTargetAtScreenPos(const Game &, const CameraState &, vector2i);
boost::shared_ptr<Cmd> makeRightclickCmd(const Game &game, vector<boost::shared_ptr<Entity>> selectedEntities, Target target);
vector2i mouseButtonToVec(sf::Event::MouseButtonEvent mEvent);

#endif // INPUT_H
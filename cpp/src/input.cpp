#include "input.h"
#include "config.h"

extern Game game;
extern UI ui;
extern vector<boost::shared_ptr<Cmd>> cmdsToSend;

UI::UI()
{
    camera.gamePos = vector2f(0, 0);
}

vector2f screenPosToGamePos(CameraState cameraState, vector2i screenPos)
{
    vector2i screenPosFromCenter = screenPos - HALF_SCREENDIM;
    vector2f result = cameraState.gamePos + vector2f(screenPosFromCenter.x, -screenPosFromCenter.y);

    return result;
}

vector2i gamePosToScreenPos(CameraState cameraState, vector2i gamePos)
{
    vector2f subtractedFromCamera = gamePos - cameraState.gamePos;
    subtractedFromCamera.y *= -1;
    vector2i result = subtractedFromCamera + HALF_SCREENDIM;
    return result;
}

vector2i mouseButtonToVec(sf::Event::MouseButtonEvent mEvent)
{
    return vector2i(mEvent.x, mEvent.y);
}

Target getTargetAtScreenPos(const Game &game, const CameraState &cameraState, vector2i screenPos)
{
    vector2f gamePos = screenPosToGamePos(cameraState, screenPos);

    boost::shared_ptr<Entity> closestValidEntity;
    float closestValidEntityDistance;
    for (unsigned int i = 0; i < game.entities.size(); i++)
    {
        boost::shared_ptr<Entity> e = game.entities[i];
        if (e)
        {
            if (e->collidesWithPoint(gamePos))
            {
                float distance = (gamePos - e->pos).getMagnitude();
                if (!closestValidEntity || distance < closestValidEntityDistance)
                {
                    closestValidEntity = e;
                    closestValidEntityDistance = distance;
                }
            }
        }
    }
    if (closestValidEntity)
        return Target(closestValidEntity->ref);
    else
        return Target(gamePos);
}

boost::shared_ptr<Cmd> makeRightclickCmd(const Game &game, vector<boost::shared_ptr<Entity>> selectedEntities, Target target)
{
    if (selectedEntities.size() == 0)
    {
        return boost::shared_ptr<Cmd>();
    }
    if (optional<vector2f> point = target.castToPoint())
    {
        return boost::shared_ptr<Cmd>(new MoveCmd(entityPtrsToRefs(selectedEntities), *point));
    }
    else if (optional<boost::shared_ptr<Entity>> entityPtrPtr = target.castToEntityPtr(game))
    {
        boost::shared_ptr<Entity> entity = *entityPtrPtr;

        // Get typechar of units if they are all of same type
        unsigned char unitTypechar = getMaybeNullEntityTypechar(selectedEntities[0]);
        bool allSameType = true;
        for (uint i = 0; i < selectedEntities.size(); i++)
        {
            if (selectedEntities[i]->typechar() != unitTypechar)
            {
                allSameType = false;
                break;
            }
        }

        if (allSameType)
        {
            if (unitTypechar == PRIME_TYPECHAR)
            {
                if (entity->typechar() == GOLDPILE_TYPECHAR)
                {
                    return boost::shared_ptr<Cmd>(new PickupCmd(entityPtrsToRefs(selectedEntities), entity->ref));
                }
                else if (entity->typechar() == GATEWAY_TYPECHAR)
                {
                    return boost::shared_ptr<Cmd>(new PushGoldThroughGatewayCmd(entityPtrsToRefs(selectedEntities), entity->ref));
                }
            }
        }
        else
        {
            // maybe in future can do a "follow" type action. Or attack if all enemies.
            return boost::shared_ptr<Cmd>();
        }
    }
    
    // couldn't cast target to a point or an entity...
    cout << "issue casting target to a point or entity in makeRightclickCmd" << endl;
    return boost::shared_ptr<Cmd>(); // return null cmd
}

// void processMiddleMouseDrag(vector2f fromScreenPos, vector2f toScreenPos)
// {
//     vector2f inGameDragVector = screenPosToGamePos(ui.camera, toScreenPos) - screenPosToGamePos(ui.camera, fromScreenPos);
//     ui.camera.pos -= inGameDragVector;
// }

// void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
// {
//     // set mouseButtonsPressed
//     ui.mouseButtonsPressed[button] = (action == GLFW_PRESS);

//     // actions
//     // press
//     if (action == GLFW_PRESS)
//     {
//         // get target
//         Target target = getTargetFromScreenPos(game, ui.camera, getGlfwClickVector2f(window));
//         // left click
//         if (button == GLFW_MOUSE_BUTTON_LEFT)
//         {
//             if (boost::shared_ptr<Entity> targetedEntity = target.castToEntityPtr(game))
//             {
//                 if (!ui.shiftPressed)
//                     ui.selectedEntities.clear();

//                 ui.selectedEntities.push_back(targetedEntity);
//                 cout << ui.selectedEntities.size() << endl;
//             }
//         }
//         // right click
//         if (button == GLFW_MOUSE_BUTTON_RIGHT)
//         {
//             queueCmdForSending(makeRightclickCmd(game, ui.selectedEntities, target));
//         }
//     }
// }
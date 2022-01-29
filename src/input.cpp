#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include "input.h"
#include "config.h"

extern Game game;
extern UI ui;
extern vector<boost::shared_ptr<Cmd>> cmdsToSend;

UI::UI()
{
    lastMouseMovePos = vector2f(0, 0);

    camera.gamePosLookAt = vector2f(0, 0);
    camera.cameraPos = glm::vec3(0, -10, 1000);

    for (uint i=0; i<8; i++)
    {
        mouseButtonsPressed[i] = false;
    }
}

glm::vec3 gamePosToGlmVec3(vector2f gamePos)
{
    return glm::vec3(gamePos.x, gamePos.y, 0.0);
}

vector2f glmVec3ToGamePos(glm::vec3 v)
{
    return vector2f(v.x, v.y);
}

vector2f screenPosToGamePos(const CameraState &camera, vector2f screenPos)
{
    glm::vec4 viewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
    glm::vec3 clickNear = glm::unProject(glm::vec3(screenPos.x, (float)WINDOW_HEIGHT - screenPos.y, 0), camera.getViewMatrix(), ProjectionMatrix, viewport);
    glm::vec3 clickFar = glm::unProject(glm::vec3(screenPos.x, (float)WINDOW_HEIGHT - screenPos.y, 1), camera.getViewMatrix(), ProjectionMatrix, viewport);

    float interp = (0 - clickNear.z) / (clickFar.z - clickNear.z);
    glm::vec3 interpolated = glm::mix(clickNear, clickFar, interp);

    return vector2f(interpolated.x, interpolated.y);
}

Target getTargetFromScreenPos(const Game &game, const CameraState &cameraState, vector2f screenPos)
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

void queueCmdForSending(boost::shared_ptr<Cmd> cmd)
{
    cmdsToSend.push_back(cmd);
}

vector2f getGlfwClickVector2f(GLFWwindow *window)
{
    double xpos, ypos;
    glfwGetCursorPos(window, &xpos, &ypos);
    return vector2f(xpos, ypos);
}

void processMiddleMouseDrag(vector2f fromScreenPos, vector2f toScreenPos)
{
    vector2f inGameDragVector = screenPosToGamePos(ui.camera, toScreenPos) - screenPosToGamePos(ui.camera, fromScreenPos);
    fprintf(stdout, "dragvec %f,%f\n", inGameDragVector.x, inGameDragVector.y);
    ui.camera.cameraPos -= gamePosToGlmVec3(inGameDragVector);
    ui.camera.gamePosLookAt -= inGameDragVector;
}

void mouseMoveCallback(GLFWwindow *window, double xpos, double ypos)
{
    vector2f newMouseMovePos = vector2f(xpos, ypos);
    if (ui.mouseButtonsPressed[GLFW_MOUSE_BUTTON_MIDDLE])
    {
        fprintf(stdout, "draggin\n");
        processMiddleMouseDrag(ui.lastMouseMovePos, newMouseMovePos);
    }

    ui.lastMouseMovePos = newMouseMovePos;
}

void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
    // set mouseButtonsPressed
    ui.mouseButtonsPressed[button] = (action == GLFW_PRESS);

    // actions
    if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS)
    {
        vector2f clickPos = getGlfwClickVector2f(window);
        Target target = getTargetFromScreenPos(game, ui.camera, clickPos);
        queueCmdForSending(makeRightclickCmd(game, ui.selectedEntities, target));
    }
}

void scrollCallback(GLFWwindow* window, double xoffset, double yoffset)
{
    vector2f scrollTargetGamePos = screenPosToGamePos(ui.camera, getGlfwClickVector2f(window));
    debugOutputVector("scrollTarget", scrollTargetGamePos);

    int remainingZoom = round(yoffset);
    while (remainingZoom != 0)
    {
        float distanceMultiplyFactor;
        bool reverseXY;
        if (remainingZoom > 0)
        {
            distanceMultiplyFactor = 1/1.1;
            remainingZoom--;
            reverseXY = false;
        }
        else
        {
            distanceMultiplyFactor = 1.1;
            remainingZoom++;
            reverseXY = true;
        }

        glm::vec3 targetPosToCamera = (ui.camera.cameraPos - gamePosToGlmVec3(scrollTargetGamePos));
        glm::vec3 newTargetPosToCamera = targetPosToCamera * distanceMultiplyFactor;
        glm::vec3 newCameraPosWithoutReverse = gamePosToGlmVec3(scrollTargetGamePos) + newTargetPosToCamera;

        glm::vec3 cameraPosMove = newCameraPosWithoutReverse - ui.camera.cameraPos;
        vector2f gameLookAtPosMove(cameraPosMove.x, cameraPosMove.y);
        if (reverseXY)
        {
            gameLookAtPosMove.x *= -1;
            gameLookAtPosMove.y *= -1;
            cameraPosMove.x *= -1;
            cameraPosMove.y *= -1;
        }
        ui.camera.cameraPos += cameraPosMove;
        ui.camera.gamePosLookAt += gameLookAtPosMove;
    }
}


void setInputCallbacks(GLFWwindow *window)
{
    glfwSetCursorPosCallback(window, mouseMoveCallback);
    glfwSetMouseButtonCallback(window, mouseButtonCallback);
    glfwSetScrollCallback(window, scrollCallback);
}
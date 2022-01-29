#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include "input.h"
#include "config.h"

extern Game game;
extern UI ui;
extern vector<boost::shared_ptr<Cmd>> cmdsToSend;

glm::vec3 gamePosToGlmVec3(vector2f gamePos)
{
    return glm::vec3(gamePos.x, gamePos.y, 0.0);
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

void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
    if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS)
    {
        vector2f clickPos = getGlfwClickVector2f(window);
        Target target = getTargetFromScreenPos(game, ui.camera, clickPos);
        queueCmdForSending(makeRightclickCmd(game, ui.selectedEntities, target));
    }
}

void scrollCallback(GLFWwindow* window, double xoffset, double yoffset)
{
    int remainingZoom = round(yoffset);
    while (remainingZoom != 0)
    {
        //by what will we multiply the current camera-to-lookpos distance?
        float distanceMultiplyFactor;
        if (remainingZoom > 0)
        {
            distanceMultiplyFactor = 1/1.1;
            remainingZoom--;
        }
        else
        {
            distanceMultiplyFactor = 1.1;
            remainingZoom++;
        }

        glm::vec3 lookAtPosToCamera = (ui.camera.cameraPos - gamePosToGlmVec3(ui.camera.gamePosLookAt));
        glm::vec3 newLookAtPosToCamera = lookAtPosToCamera * distanceMultiplyFactor;
        ui.camera.cameraPos = gamePosToGlmVec3(ui.camera.gamePosLookAt) + newLookAtPosToCamera;
    }
}

void setInputCallbacks(GLFWwindow *window)
{
    glfwSetMouseButtonCallback(window, mouseButtonCallback);
    glfwSetScrollCallback(window, scrollCallback);
}
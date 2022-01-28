#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include "input.h"
#include "config.h"

extern UI ui;

vector2f screenPosToGroundPos(const CameraState &camera, vector2f screenPos)
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
    vector2f gamePos = screenPosToGroundPos(cameraState, screenPos);

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

vector2f getGlfwClickVector2f(GLFWwindow *window)
{
    double xpos, ypos;
    glfwGetCursorPos(window, &xpos, &ypos);
    return vector2f(xpos, ypos);
}

void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
    {
        vector2f clickPos = getGlfwClickVector2f(window);
        screenPosToGroundPos(ui.camera, clickPos);
    }
}

void setInputCallbacks(GLFWwindow *window)
{
    glfwSetMouseButtonCallback(window, mouseButtonCallback);
}
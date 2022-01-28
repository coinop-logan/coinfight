#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include "myvectors.h"
#include "common.h"
#include "graphics.h"
#include "cmds.h"

#ifndef INPUT_H
#define INPUT_H

struct UI
{
    vector<boost::shared_ptr<Entity>> selectedEntities;
    CameraState camera;
};
vector2f screenPosToGroundPos(const CameraState &, vector2f );
Target getTargetFromScreenPos(const Game &, const CameraState &, vector2f );
boost::shared_ptr<Cmd> makeRightclickCmd(const Game &game, vector<boost::shared_ptr<Entity>> selectedEntities, Target target);
vector2f getGlfwClickVector2f(GLFWwindow *);
void mouseButtonCallback(GLFWwindow *, int button, int action, int mods);
void setInputCallbacks(GLFWwindow *);

#endif // INPUT_H
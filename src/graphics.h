#include <boost/shared_ptr.hpp>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "engine.h"

#ifndef GRAPHICS_H
#define GRAPHICS_H

struct CameraState
{
    vector2f gamePosLookAt;
    glm::vec3 cameraPos;
};
GLuint loadShaders(const char * vertex_file_path, const char * fragment_file_path);
bool loadOBJ(const char * path, vector<glm::vec3> &outVertices, vector<glm::vec2> &outUVs, vector<glm::vec3> &outNormals);
GLFWwindow* setupGraphics();
void display(GLFWwindow*, const Game &, const CameraState & );
void cleanupGraphics();

#endif // GRAPHICS_H
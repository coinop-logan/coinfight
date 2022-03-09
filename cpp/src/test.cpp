#include <iostream>
#include <boost/lexical_cast.hpp>
#include <optional>
#include <boost/filesystem.hpp>
#include <typeinfo>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include "sigWrapper.h"
// #include "engine.h"
#include "common.h"
// #include "config.h"
// #include "vchpack.h"
// #include "myvectors.h"
// #include "coins.h"

using namespace std;

// void makeSure(bool condition) // hacky test function
// {
//     cout << (condition ? "PASSED" : "FAILED") << endl;
// }

// void makeSure(string name, bool condition) // hacky test function
// {
//     cout << (condition ? "PASSED: " : "FAILED: ") << name << endl;
// }

// glm::vec3 playerAddressToGlmColor(string address)
// {
//     float vals[3];
//     for (uint i=0; i<3; i++)
//     {
//         string charStr = address.substr(2 + i, 1);
//         unsigned int intVal = std::stoul(charStr, nullptr, 16);
//         vals[i] = intVal / 15.0;
//     }
//     return glm::vec3(vals[0], vals[1], vals[2]);
// }

int main()
{
    // glm::vec3 color = playerAddressToGlmColor("0xf3f000");
    // cout << color.x << "," << color.y << "," << color.z << endl;

    return 0;
}
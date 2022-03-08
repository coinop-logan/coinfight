#include <iostream>
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


int main()
{
    string msg("hi");
    string sig("f9ec9c22e31782f5397c8ee1ec63fed83aad060c0a0899b1dc5562348728bb196974bbd5d1fe24ff665228becaa051b57413a3a7f6969cc992fe1b72e1bc2c5d1c");

    string result = signedMsgToAddress(msg, sig);

    cout << "result: " << result << endl;

    return 0;
}
#include <iostream>
#include <optional>
#include <boost/filesystem.hpp>
#include <typeinfo>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
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
    boost::filesystem::path accountingDirPath("./accounting/pending_deposits/");
    boost::filesystem::directory_iterator directoryEndIter; // default constructor makes it an end_iter

    for (boost::filesystem::directory_iterator dirIter(accountingDirPath); dirIter != directoryEndIter; dirIter++)
    {
        if (boost::filesystem::is_regular_file(dirIter->path())) {
            string current_file = dirIter->path().string();
            cout << current_file << endl;
        }
    }

    return 0;
}
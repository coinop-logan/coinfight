using namespace std;

#include "input.h"
#include "graphics.h"

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
    CameraState s;
    s.gamePos = vector2f(0,0);
    vector2f v(10, 10);
    debugOutputVector("result", screenPosToGamePos(s, v));
    
    return 0;
}
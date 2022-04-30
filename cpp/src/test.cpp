#include <iostream>
#include <optional>
#include "engine.h"
#include "myvectors.h"
#include "fixed.h"

// void makeSure(bool condition) // hacky test function
// {
//     cout << (condition ? "PASSED" : "FAILED") << endl;
// }

// void makeSure(string name, bool condition) // hacky test function
// {
//     cout << (condition ? "PASSED: " : "FAILED: ") << name << endl;
// }

using namespace std;

int main()
{
    vch dest;

    dest.clear();
    float a = -241.59759;
    cout << "original: " << a << endl;
    packToVch(&dest, "d", a);
    float a2;
    unpackFromIter(dest.begin(), "d", &a2);
    cout << "result: " << a2 << endl;
    
    return 0;
}
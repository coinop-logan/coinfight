#include <iostream>
#include <optional>
#include "engine.h"
#include "myvectors.h"

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
    SearchGrid sGrid;

    auto nearby = sGrid.entitiesNearGamePosSloppy(vector2f(10,10), 500);
    
    return 0;
}
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
    optional<vector2i> cell = sGrid.gamePosToCell(vector2f(-0.1, -0.1));

    if (cell)
        cout << cell->x << "," << cell->y << endl;
    else
        cout << "no cell!" << endl;
    
    return 0;
}
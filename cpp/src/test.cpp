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

    Game game;
    
    boost::shared_ptr<Entity> fighter(new Fighter(&game, 1, 0, vector2f(0,0)));
    sGrid.registerEntityCell(fighter);
    fighter->pos = vector2f(25, 25);
    sGrid.updateEntityCell(fighter);
    
    return 0;
}
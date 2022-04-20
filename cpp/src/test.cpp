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

    for (int i = 0; i<10; i++)
    {
        boost::shared_ptr<Entity> fighter(new Fighter(&game, i+1, 0, vector2f(i*4,i*4)));
        sGrid.registerEntityCell(fighter);
    }

    auto nearby = sGrid.entitiesNearGamePosSloppy(vector2f(10,10), 500);
    
    return 0;
}
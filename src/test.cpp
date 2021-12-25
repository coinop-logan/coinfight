#include <iostream>
#include "engine.h"
#include "config.h"
#include "myvectors.h"


int main()
{
    GoldPile gp(NULL, NULL_ENTITYREF, vector2f(0,0), 50);

    std::cout << gp.tryAddAmount(50) << std::endl;

    return 0;
}
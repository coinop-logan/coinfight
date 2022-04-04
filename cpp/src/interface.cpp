#include "interface.h"

void updateScreenDimensions(vector2i newDimensions)
{
    screenDimensions = newDimensions;
    screenCenter = screenDimensions / 2;
}
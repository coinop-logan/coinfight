#include "interface.h"

vector2i screenDimensions, screenCenter;

void updateScreenDimensions(vector2i newDimensions)
{
    screenDimensions = newDimensions;
    screenCenter = screenDimensions / 2;
}
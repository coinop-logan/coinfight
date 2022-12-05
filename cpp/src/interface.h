#include "myvectors.h"

#ifndef INTERFACE_H
#define INTERFACE_H

struct CameraState
{
    vector2i gamePos;
};

extern vector2i screenDimensions, screenCenter;

void updateScreenDimensions(vector2i newDimensions);

#endif
#include <iostream>
#include "common.h"

using namespace std;

void packTypechar(vch *dest, unsigned char typechar)
{
    packToVch(dest, "C", typechar);
}

void debugOutputVch(vch out)
{
    cout << "0x";
    for (unsigned int i = 0; i < out.size(); i++)
    {
        cout << hex << (unsigned int)(out[i]);
    }
}

void prependVchWithSize(vch *vchDest)
{
    uint64_t packetSize = vchDest->size();
    vch sizeData;
    packToVch(&sizeData, "Q", packetSize);

    vchDest->insert(vchDest->begin(), sizeData.begin(), sizeData.end());
}

void packVector2f(vch *destVch, const vector2f &v)
{
    packToVch(destVch, "ff", v.x, v.y);
}

vchIter unpackVector2f(vchIter src, vector2f *v)
{
    return unpackFromIter(src, "ff", &v->x, &v->y);
}

vchIter unpackTypecharFromIter(vchIter src, unsigned char *typechar)
{
    return unpackFromIter(src, "C", typechar);
}

void packEntityRef(vch *destVch, EntityRef id)
{
    packToVch(destVch, "H", id);
}
vchIter unpackEntityRef(vchIter iter, EntityRef *id)
{
    return unpackFromIter(iter, "H", id);
}

bool entityRefIsNull(EntityRef id)
{
    return id == 0;
}
#include <iostream>
#include <stdio.h>
#include "common.h"
#include "coins.h"

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
void debugOutputVector(const char *label, vector2f v)
{
    fprintf(stdout, label);
    fprintf(stdout, ": %f,%f\n", v.x, v.y);
}
void debugOutputVector(const char *label, vector3f v)
{
    fprintf(stdout, label);
    fprintf(stdout, ": %f,%f,%f\n", v.x, v.y, v.z);
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

void packEntityRef(vch *destVch, EntityRef ref)
{
    packToVch(destVch, "H", ref);
}
vchIter unpackEntityRef(vchIter iter, EntityRef *ref)
{
    return unpackFromIter(iter, "H", ref);
}

void packStringToVch(std::vector<unsigned char> *vch, string s)
{
    char* cstr;
    cstr = &s[0];
    packToVch(vch, "s", cstr);
}
vchIter unpackStringFromIter(vchIter iter, uint16_t maxSize, string *s)
{
    char formatStr[7];
    snprintf(formatStr, sizeof(formatStr), "%ds", maxSize);

    char cstr[maxSize+1];
    vchIter newIter = unpackFromIter(iter, formatStr, cstr);
    *s = string(cstr);

    return newIter;
}

bool entityRefIsNull(EntityRef ref)
{
    return ref == 0;
}

std::optional<unsigned int> safeUIntAdd(unsigned int a, unsigned int b)
{
    unsigned int sum = a + b;
    if (sum < a) {
        return {};
    }
    else {
        return {sum};
    }
}

coinsInt dollarsToCoinsInt(float dollars)
{
    return dollars * (pow(10, CREDIT_PER_DOLLAR_EXPONENT));
}
float coinsIntToDollars(coinsInt coins)
{
    return coins / (pow(10, CREDIT_PER_DOLLAR_EXPONENT));
}

float degToRad(float deg)
{
    return deg / (180 / M_PI);
}
float radToDeg(float rad)
{
    return rad * (180 / M_PI);
}

vector2f randomVectorWithMagnitude(float magnitude)
{
    float angle = ((double)rand() / RAND_MAX) * M_PI * 2;
    return composeVector2f(angle, magnitude);
}

vector2f randomVectorWithMagnitudeRange(float min, float max)
{
    float magnitude = (((double)rand() / RAND_MAX) * (max - min)) + max;
    return randomVectorWithMagnitude(magnitude);
}
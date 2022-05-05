#include <iostream>
#include <stdio.h>
#include "common.h"
#include "coins.h"

using namespace std;

void debugOutputVector(const char *label, vector2fp v)
{
    fprintf(stdout, "%s", label);
    fprintf(stdout, ": %f,%f\n", v.x, v.y);
}
void debugOutputVector(const char *label, vector2fl v)
{
    fprintf(stdout, "%s", label);
    fprintf(stdout, ": %f,%f\n", v.x, v.y);
}
void debugOutputVector(const char *label, vector3f v)
{
    fprintf(stdout, "%s", label);
    fprintf(stdout, ": %f,%f,%f\n", v.x, v.y, v.z);
}

void packFixed32(Netpack::Builder* to, fixed32 val)
{
    to->packInt32_t(val.raw_value());
}
fixed32 consumeFixed32(Netpack::Consumer *from)
{
    return fixed32::from_raw_value(from->consumeInt32_t());
}

void packVector2fp(Netpack::Builder* to, vector2fp v)
{
    packFixed32(to, v.x);
    packFixed32(to, v.y);
}
vector2fp consumeVector2fp(Netpack::Consumer *from)
{
    fixed32 x = consumeFixed32(from);
    fixed32 y = consumeFixed32(from);
    return vector2fp(x, y);
}


void packEntityRef(Netpack::Builder* to, EntityRef ref)
{
    to->packUint16_t(ref);
}
EntityRef consumeEntityRef(Netpack::Consumer *from)
{
    return from->consumeUint16_t();
}

void packTypechar(Netpack::Builder* to, uint8_t typechar)
{
    to->packUint8_t(typechar);
}
uint8_t consumeTypechar(Netpack::Consumer* from)
{
    return from->consumeUint8_t();
}

coinsInt dollarsToCoinsIntND(float dollars)
{
    return dollars * (pow(10, CREDIT_PER_DOLLAR_EXPONENT));
}
float coinsIntToDollarsND(coinsInt coins)
{
    return coins / (pow(10, CREDIT_PER_DOLLAR_EXPONENT));
}

vector2fl randomVectorWithMagnitude(float magnitude)
{
    float angle = ((double)rand() / RAND_MAX) * M_PI * 2;
    return composeVector2fl(angle, magnitude);
}

vector2fl randomVectorWithMagnitudeRange(float min, float max)
{
    float magnitude = (((double)rand() / RAND_MAX) * (max - min)) + min;
    return randomVectorWithMagnitude(magnitude);
}

sf::Vector2f toSFVec(vector2fl v)
{
    return sf::Vector2f(v.x, v.y);
}
sf::Vector2i toSFVec(vector2i v)
{
    return sf::Vector2i(v.x, v.y);
}
vector2fl fromSFVec(sf::Vector2f v)
{
    return vector2fl(v.x, v.y);
}
vector2i fromSFVec(sf::Vector2i v)
{
    return vector2i(v.x, v.y);
}

uint32_t floorSquareFixed(fixed32 val)
{
    int32_t floored = static_cast<int32_t>(val);
    return (floored * floored);
}
uint32_t newtonSqrtFloor(uint32_t x) {
    if (x==0) return x;
    uint32_t dividend = x;
    uint32_t val = x;
    uint32_t last;
    do {
        last = val;
        val = (val + dividend / val) / 2;
    } while(val - last != 0);
    return (int)val;        
}
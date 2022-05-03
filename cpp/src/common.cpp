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

void packFixed32(Netpack::Builder *dest, fixed32 val)
{
    dest->packInt32_t(val.raw_value());
}
fixed32 consumeFixed32(Netpack::Consumer *from)
{
    return fixed32::from_raw_value(from->consumeInt32_t());
}

void packVector2fp(Netpack::Builder *dest, const vector2fp &v)
{
    packFixed32(dest, v.x);
    packFixed32(dest, v.y);
}
vector2fp consumeVector2f(Netpack::Consumer *from)
{
    return vector2fp(consumeFixed32(from), consumeFixed32(from));
}


void packEntityRef(Netpack::Builder *dest, EntityRef ref)
{
    dest->packUint16_t(ref);
}
EntityRef consumeEntityRef(Netpack::Consumer *from)
{
    return from->consumeUint16_t();
}

// std::optional<unsigned int> safeUIntAdd(unsigned int a, unsigned int b)
// {
//     unsigned int sum = a + b;
//     if (sum < a) {
//         return {};
//     }
//     else {
//         return {sum};
//     }
// }

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
vector2fl fromSFVec(sf::Vector2f v)
{
    return vector2fl(v.x, v.y);
}
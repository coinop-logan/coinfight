#include <stdint.h>
#include <optional>
#include <boost/shared_ptr.hpp>
#include "myvectors.h"
#include "netpack.h"
#include "fpm/fixed.hpp"
#include "coins.h"

#ifndef COMMON_H
#define COMMON_H

using namespace std;

void debugOutputVector(const char *,vector2fp);
void debugOutputVector(const char *,vector2fl);
void debugOutputVector(const char *,vector3f);

void packFixed32(Netpack::Builder *dest, fixed32 val);
fixed32 consumeFixed32(Netpack::Consumer *from);

void packVector2fp(Netpack::Builder *dest, const vector2fp &v);
vector2fp consumeVector2f(Netpack::Consumer *from);

void packEntityRef(Netpack::Builder *dest, EntityRef ref);
EntityRef consumeEntityRef(Netpack::Consumer *from);

// std::optional<unsigned int> safeUIntAdd(unsigned int, unsigned int);

coinsInt dollarsToCoinsInt(float dollars);
float coinsIntToDollars(coinsInt coins);

float degToRad(float);
float radToDeg(float);

vector2fl randomVectorWithMagnitude(float magnitude);
vector2fl randomVectorWithMagnitudeRange(float min, float max);

template<class T, class U> vector<boost::shared_ptr<T>> filterForType(vector<boost::shared_ptr<U>> v)
{
    vector<boost::shared_ptr<T>> filtered;
    for (unsigned int i=0; i<v.size(); i++)
    {
        if (boost::shared_ptr<T> found = boost::dynamic_pointer_cast<T, U>(v[i]))
        {
            filtered.push_back(found);
        }
    }
    return filtered;
}

template<class T, class U> vector<boost::shared_ptr<U>> filterForTypeKeepContainer(vector<boost::shared_ptr<U>> v)
{
    vector<boost::shared_ptr<U>> filtered;
    for (unsigned int i=0; i<v.size(); i++)
    {
        if (boost::shared_ptr<T> found = boost::dynamic_pointer_cast<T, U>(v[i]))
        {
            filtered.push_back(found);
        }
    }
    return filtered;
}

sf::Vector2f toSFVec(vector2fl v);
vector2fl fromSFVec(sf::Vector2f v);

#endif // COMMON_H

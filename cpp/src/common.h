#include <stdint.h>
#include <optional>
#include <boost/shared_ptr.hpp>
#include "myvectors.h"
#include "vchpack.h"
#include "coins.h"

#ifndef COMMON_H
#define COMMON_H

using namespace std;

using vch = vector<unsigned char>;
using vchIter = vector<unsigned char>::iterator;

void packTypechar(vch *dest, unsigned char typechar);

void debugOutputVch(vch);
void debugOutputVector(const char *,vector2f);
void debugOutputVector(const char *,vector3f);

void prependVchWithSize(vch *vchDest);

void packVector2f(vch *destVch, const vector2f &v);
vchIter unpackVector2f(vchIter src, vector2f *v);

vchIter unpackTypecharFromIter(vchIter src, unsigned char *typechar);

void packEntityRef(vch *destVch, EntityRef ref);
vchIter unpackEntityRef(vchIter iter, EntityRef *ref);

void packStringToVch(std::vector<unsigned char> *vch, string s);
vchIter unpackStringFromIter(vchIter iter, uint16_t maxSize, string *s);

bool entityRefIsNull(EntityRef);
std::optional<unsigned int> safeUIntAdd(unsigned int, unsigned int);

coinsInt dollarsToCoinsInt(float dollars);
float coinsIntToDollars(coinsInt coins);

float degToRad(float);
float radToDeg(float);

vector2f randomVectorWithMagnitude(float magnitude);
vector2f randomVectorWithMagnitudeRange(float min, float max);

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

sf::Vector2f toSFVec(vector2f v);
vector2f fromSFVec(sf::Vector2f v);

#endif // COMMON_H

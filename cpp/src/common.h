#include <stdint.h>
#include <optional>
#include <boost/shared_ptr.hpp>
#include "address.h"
#include "myvectors.h"
#include "netpack.h"
#include "fpm/fixed.hpp"
#include "coins.h"

#ifndef COMMON_H
#define COMMON_H

using namespace std;

const int EXPECTED_SIGNATURE_LENGTH = 130;

void debugOutputVector(const char *,vector2fp);
void debugOutputVector(const char *,vector2fl);
void debugOutputVector(const char *,vector3f);

void packFixed32(Netpack::Builder* to, fixed32 val);
fixed32 consumeFixed32(Netpack::Consumer *from);

void packVector2fp(Netpack::Builder* to, vector2fp v);
vector2fp consumeVector2fp(Netpack::Consumer *from);

void packEntityRef(Netpack::Builder* to, EntityRef ref);
EntityRef consumeEntityRef(Netpack::Consumer* from);

void packTypechar(Netpack::Builder* to, uint8_t typechar);
uint8_t consumeTypechar(Netpack::Consumer* from);

coinsInt dollarsToCoinsIntND(float dollars);
float coinsIntToDollarsND(coinsInt coins);

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
sf::Vector2i toSFVec(vector2i v);
sf::Vector2f toSFVecF(vector2i v);
vector2fl fromSFVec(sf::Vector2f v);
vector2i fromSFVec(sf::Vector2i v);

uint32_t floorSquareFixed(fixed32);
uint32_t newtonSqrtFloor(uint32_t x);
uint64_t constexpr ipow(uint32_t base, uint32_t exp)
{
    uint64_t result = 1;
    for (;;)
    {
        if (exp & 1)
            result *= base;
        exp >>= 1;
        if (!exp)
            break;
        base *= base;
    }

    return result;
}

vector<string> splitLineIntoWords(string);

optional<tuple<Address, string>> decodeAddressAndSig(string str);
optional<Address> validateAddress(string addrStr);

#endif // COMMON_H
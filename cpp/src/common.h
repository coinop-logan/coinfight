#include <stdint.h>
#include <optional>
#include <boost/shared_ptr.hpp>
#include <boost/random.hpp>
#include "address.h"
#include "myvectors.h"
#include "netpack.h"
#include "fpm/fixed.hpp"
#include "coins.h"

#ifndef COMMON_H
#define COMMON_H

using namespace std;

typedef boost::mt11213b baseRandGenType;

const int EXPECTED_SIGNATURE_LENGTH = 130;

const sf::Color NEUTRAL_TEAM_COLOR(150, 150, 150);

void debugOutputVector(const char *,vector2i);
void debugOutputVector(const char *,vector2fp);
void debugOutputVector(const char *,vector2fl);
void debugOutputVector(const char *,vector3f);

void packRandGenerator(Netpack::Builder* to, baseRandGenType randGen);
baseRandGenType consumeRandGenerator(Netpack::Consumer* from);

void packTimeT(Netpack::Builder* to, time_t timeVal);
time_t consumeTimeT(Netpack::Consumer* from);

void packFixed32(Netpack::Builder* to, fixed32 val);
fixed32 consumeFixed32(Netpack::Consumer *from);

void packVector2fp(Netpack::Builder* to, vector2fp v);
vector2fp consumeVector2fp(Netpack::Consumer *from);

void packEntityRef(Netpack::Builder* to, EntityRef ref);
EntityRef consumeEntityRef(Netpack::Consumer* from);

void packTypechar(Netpack::Builder* to, uint8_t typechar);
uint8_t consumeTypechar(Netpack::Consumer* from);

coinsInt bcCurrencyAmountToCoinsIntND(float dollars);
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

sf::Vector2f toSFVecF(vector2fl v);
sf::Vector2i toSFVecI(vector2i v);
sf::Vector2f toSFVecF(vector2i v);
vector2fl fromSFVec(sf::Vector2f v);
vector2i fromSFVec(sf::Vector2i v);
vector2i fromSFVec(sf::Vector2u v);

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

double radiansToDegrees(double radians);
vector2i getScreenSize(sf::RenderWindow* window);
vector2fl getCurrentViewSize(sf::RenderWindow* window);
vector2fl getViewSize(sf::RenderWindow* window, sf::View view);
string uint16ToString(uint16_t x);
string floatToPercentString(float x);
string floatToShortPercentString(float x);
template<typename C>
void forEachStartAt(vector<C>* vec, unsigned int start, void (*actionFunc)(C))
{
    if (vec->size() == 0)
    {
        if (start > 0)
        {
            throw runtime_error("Start position out of range of vector");
        }
        return;
    }

    if (start >= vec->size())
    {
        throw runtime_error("Start position out of range of vector");
    }

    unsigned int i = start;
    bool firstLoop = true;
    while (true)
    {
        if (!firstLoop && i == start)
        {
            break;
        }

        actionFunc((*vec)[i]);

        if (firstLoop) firstLoop = false;
        i ++;
        if (i == vec->size())
        {
            i = 0;
        }
    }
}

struct GameSettings
{
    coinsInt
        goldTransferRate,
        beaconBuildRate;
    
    coinsInt primeGoldCapacity;
    
    coinsInt
        gatewayOrBeaconCost,
        primeCost,
        fighterCost,
        turretCost;
    
    uint16_t
        beaconHealth,
        gatewayHealth,
        primeHealth,
        fighterHealth,
        turretHealth;

    GameSettings() {}
    void pack(Netpack::Builder*);
    GameSettings(Netpack::Consumer*);
};

GameSettings defaultGameSettings();

#endif // COMMON_H
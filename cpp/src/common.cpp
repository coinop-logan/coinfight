#include <iostream>
#include <stdio.h>
#include <fpm/ios.hpp>
#include <boost/algorithm/string.hpp>
#include "common.h"
#include "coins.h"
#include "nlohmann/json.hpp"

using json = nlohmann::json;

using namespace std;

void debugOutputVector(const char *label, vector2i v)
{
    fprintf(stdout, "%s: ", label);
    cout << v.x << "," << v.y << endl;
}
void debugOutputVector(const char *label, vector2fp v)
{
    fprintf(stdout, "%s: ", label);
    cout << v.x << "," << v.y << endl;
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


void packRandGenerator(Netpack::Builder* to, baseRandGenType randGen)
{
    stringstream ss;
    ss << randGen;
    to->packStringWith16bitSize(ss.str());
}
baseRandGenType consumeRandGenerator(Netpack::Consumer* from)
{
    string s = from->consumeStringWith16bitSize();
    stringstream ss(s);
    baseRandGenType randGen;
    ss >> randGen;
    return randGen;
}
void packTimeT(Netpack::Builder* to, time_t timeVal)
{
    to->packUint64_t(timeVal);
}
time_t consumeTimeT(Netpack::Consumer* from)
{
    return from->consumeUint64_t();
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

coinsInt bcCurrencyAmountToCoinsIntND(float bcCurrencyAmount)
{
    return bcCurrencyAmount * (pow(10, LOCAL_CREDIT_EXPONENT));
}
float coinsIntToDollarsND(coinsInt coins)
{
    return coins / (pow(10, LOCAL_CREDIT_EXPONENT));
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

sf::Vector2f toSFVecF(vector2fl v)
{
    return sf::Vector2f(v.x, v.y);
}
sf::Vector2i toSFVecI(vector2i v)
{
    return sf::Vector2i(v.x, v.y);
}
sf::Vector2f toSFVecF(vector2i v)
{
    return sf::Vector2f(v.x, v.y);
}
vector2fl fromSFVec(sf::Vector2f v)
{
    return vector2fl(v.x, v.y);
}
vector2i fromSFVec(sf::Vector2i v)
{
    return vector2i(v.x, v.y);
}
vector2i fromSFVec(sf::Vector2u v)
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
    int64_t lastChange;
    while (true) {
        lastChange = int64_t(val) - last;
        last = val;
        val = (val + dividend / val) / 2;
        if (val - last == (-lastChange))
            return min(val, last);
    }
}

vector<string> splitLineIntoWords(string line)
{
    vector<string> words;
    boost::split(words, line, boost::is_any_of(" "));

    return words;
}

optional<tuple<Address, string>> decodeAddressAndSig(string str)
{
    auto data = json::parse(str, nullptr, false);
    if (data.is_discarded())
    {
        return {};
    }

    if (!(data.contains("sig") && data.contains("address")))
    {
        return {};
    }

    return {{Address(data["address"]), data["sig"]}};
}

optional<Address> validateAddress(string addrStr)
{
    if (addrStr.substr(0, 2) != "0x")
        return {};
    if (addrStr.size() != 42)
        return {};
    for (unsigned int i=2; i<addrStr.size(); i++)
    {
        if (!isxdigit(addrStr[i]))
        {
            return {};
        }
    }
    
    return Address(addrStr);
}

double radiansToDegrees(double radians)
{
    return (radians / M_PI) * 180;
}

vector2i getScreenSize(sf::RenderWindow* window)
{
    return fromSFVec(window->getSize());
}
vector2fl getCurrentViewSize(sf::RenderWindow* window)
{
    return fromSFVec(window->getView().getSize());
}
vector2fl getViewSize(sf::RenderWindow* window, sf::View view)
{
    sf::View currentView = window->getView();

    window->setView(view);
    vector2fl size = getCurrentViewSize(window);

    window->setView(currentView);

    return size;
}

string uint16ToString(uint16_t x) {
    stringstream ss;
    ss << (unsigned int)x;
    return ss.str();
}

string floatToPercentString(float x)
{
    char buf[10];
    snprintf(buf, 10, "$%.2f", x*100);
    return string(buf) + "%";
}
string floatToShortPercentString(float x)
{
    int roundedPercent = round(x*100);
    char buf[10];
    snprintf(buf, 10, "%i", roundedPercent);
    return string(buf) + "%";
}

void GameSettings::pack(Netpack::Builder* builder)
{
    packCoinsInt(builder, goldTransferRate);
    packCoinsInt(builder, beaconBuildRate);

    packCoinsInt(builder, gatewayOrBeaconCost);

    builder->packUint16_t(beaconHealth);

    builder->packUint16_t(gatewayHealth);

    packCoinsInt(builder, primeCost);
    builder->packUint16_t(primeHealth);

    packCoinsInt(builder, fighterCost);
    builder->packUint16_t(fighterHealth);

    packCoinsInt(builder, turretCost);
    builder->packUint16_t(turretHealth);
}
GameSettings::GameSettings(Netpack::Consumer* consumer)
{
    goldTransferRate = consumeCoinsInt(consumer);
    beaconBuildRate = consumeCoinsInt(consumer);

    gatewayOrBeaconCost = consumeCoinsInt(consumer);

    beaconHealth = consumer->consumeUint16_t();

    gatewayHealth = consumer->consumeUint16_t();

    primeCost = consumeCoinsInt(consumer);
    primeHealth = consumer->consumeUint16_t();

    fighterCost = consumeCoinsInt(consumer);
    fighterHealth = consumer->consumeUint16_t();

    turretCost = consumeCoinsInt(consumer);
    turretHealth = consumer->consumeUint16_t();
}

GameSettings defaultGameSettings()
{
    GameSettings settings;

    settings.goldTransferRate = 60;
    settings.beaconBuildRate = 100;

    settings.gatewayOrBeaconCost = 500000;

    settings.beaconHealth = 500;

    settings.gatewayHealth = 1500;

    settings.primeCost = 50000;
    settings.primeHealth = 100;

    settings.fighterCost = 200000;
    settings.fighterHealth = 300;

    settings.turretCost = 1000000;
    settings.turretHealth = 900;

    return settings;
}

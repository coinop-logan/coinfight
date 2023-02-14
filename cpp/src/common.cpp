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
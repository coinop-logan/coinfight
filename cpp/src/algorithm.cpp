#include <chrono>
#include <iomanip>
#include <sstream>
#include "algorithm.h"

std::string lowercaseStr(std::string s)
{
    std::transform(s.begin(), s.end(), s.begin(),
    [](unsigned char c){ return std::tolower(c); });
    return s;
}

sf::Color playerAddressToColor(Address address)
{
    uint8_t vals[3];
    for (uint8_t i=0; i<3; i++)
    {
        string charStr = address.getString().substr(2 + i, 1);
        uint8_t intVal = std::stoul(charStr, nullptr, 16);
        vals[i] = 55 + ((intVal / 15.0) * 200);
    }
    return sf::Color(vals[0], vals[1], vals[2]);
}

string unixTimeToHumanReadableTimeString(time_t unixTime)
{
    char timeBuf[80];
    struct tm ts;
    ts = *localtime(&unixTime);
    strftime(timeBuf, sizeof(timeBuf), "%a %Y-%m-%d %H:%M:%S %Z", &ts);
    return string(timeBuf);
}

string unixTimeToTimeLeftString(time_t unixTime)
{
    using namespace chrono;

    // not sure why this tweak is necessary, but...
    time_t tweakedUnixTime = unixTime + 1;

    system_clock::time_point start = system_clock::from_time_t(tweakedUnixTime);
    system_clock::time_point now = system_clock::now();
    auto timeLeftDuration = start - now;

    int ddd = duration_cast<hours>(timeLeftDuration).count() / 24;
    int hh = duration_cast<hours>(timeLeftDuration).count() % 24;
    int mm = duration_cast<minutes>(timeLeftDuration).count() % 60;
    int ss = duration_cast<seconds>(timeLeftDuration).count() % 60;
    int msec = duration_cast<milliseconds>(timeLeftDuration).count() % 1000;

    std::ostringstream stream;
    
    if (ddd > 0)
        stream << ddd << " days, ";
    stream <<
        std::setfill('0') << std::setw(2) << hh << ':' <<
        std::setfill('0') << std::setw(2) << mm << ':' << 
        std::setfill('0') << std::setw(2) << ss << '.' <<
        std::setfill('0') << std::setw(1) << (msec / 100);
    std::string result = stream.str();

    return result;
}
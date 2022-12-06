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
#include "algorithm.h"

std::string lowercaseStr(std::string s)
{
    std::transform(s.begin(), s.end(), s.begin(),
    [](unsigned char c){ return std::tolower(c); });
    return s;
}
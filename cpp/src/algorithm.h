#include <string>
#include <boost/algorithm/string.hpp>
#include <SFML/Graphics.hpp>

#include "address.h"

#ifndef ALGORITHM_H
#define ALGORITHM_H

std::string lowercaseStr(std::string s);
sf::Color playerAddressToColor(Address address);

#endif // ALGORITHM_H
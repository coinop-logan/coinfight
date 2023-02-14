#include <SFML/Graphics.hpp>
#include "common/myvectors.h"

#ifndef COMMON_UTILS_H
#define COMMON_UTILS_H

sf::Vector2f toSFVecF(vector2fl v);
sf::Vector2i toSFVecI(vector2i v);
sf::Vector2f toSFVecF(vector2i v);
vector2fl fromSFVec(sf::Vector2f v);
vector2i fromSFVec(sf::Vector2i v);
vector2i fromSFVec(sf::Vector2u v);

#endif // COMMON_UTILS_H
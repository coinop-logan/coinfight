#include <SFML/Graphics.hpp>
#include "common/myvectors.h"

#ifndef INTERFACE_COMMON_H
#define INTERFACE_COMMON_H

vector2i getScreenSize(sf::RenderWindow* window);
vector2fl getCurrentViewSize(sf::RenderWindow* window);
vector2fl getViewSize(sf::RenderWindow* window, sf::View view);

#endif // INTERFACE_COMMON_H
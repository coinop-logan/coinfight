#include <SFML/Graphics.hpp>

#ifndef INTERFACE_GRAPHICS_COMMON_H
#define INTERFACE_GRAPHICS_COMMON_H

void displayTitle(sf::RenderWindow*, int yPadding);
void loadFonts();
sf::Font* getHumanFont();
sf::Font* getFWFont();

#endif // INTERFACE_GRAPHICS_COMMON_H
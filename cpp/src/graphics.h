#include <SFML/Graphics.hpp>
#include <boost/shared_ptr.hpp>
#include "engine.h"
#include "input.h"
#include "tutorial.h"

#ifndef GRAPHICS_H
#define GRAPHICS_H

const vector2fl COMBATUNIT_SHOT_OFFSET(20, 10);

const vector2i UNIT_INFO_BOX_SIZE(350, 350);
const vector2i UX_ELEMENT_SPACING(5, 5);
const vector2i UX_BOX_PADDING(10, 10);
const sf::Color UX_BOX_BORDER_COLOR(150, 150, 200);
const sf::Color UX_BOX_BACKGROUND_COLOR(0, 0, 0, 200);

const unsigned int TITLE_POS_Y = 50;

sf::RenderWindow* setupGraphics(bool fullscreen, bool smallScreen);
void display(sf::RenderWindow *window, Game *game, GameUI* ui, optional<Address> maybePlayerAddress, Tutorial*, sf::Font mainFont, sf::Font tutorialFont, bool drawWalletHints);
void loadFonts(sf::Font* mainFont, sf::Font* tutorialFont);
void cleanupGraphics(sf::RenderWindow* window);
void displayTitle(sf::RenderWindow*, sf::Font*);

#endif // GRAPHICS_H
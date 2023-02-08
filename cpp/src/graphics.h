#include <SFML/Graphics.hpp>
#include <boost/shared_ptr.hpp>
#include "engine.h"
#include "input.h"
#include "tutorial.h"

#ifndef GRAPHICS_H
#define GRAPHICS_H

const vector2fl COMBATUNIT_SHOT_OFFSET(20, 10);

const unsigned int TITLE_POS_Y = 50;

const vector2i KEYBUTTONHINT_SIZE(400, 200);

sf::RenderWindow* setupGraphics(bool fullscreen, bool smallScreen);
void display(sf::RenderWindow *window, Game *game, GameUI* ui, optional<Address> maybePlayerAddress, Tutorial*, sf::Font* mainFont, sf::Font* tutorialFont, bool drawWalletHints);
void loadFonts(sf::Font* mainFont, sf::Font* tutorialFont);
void cleanupGraphics(sf::RenderWindow* window);
void displayTitle(sf::RenderWindow*, sf::Font*);
void loadKeyCommandIcons();
sf::Sprite* getSpriteForKeyButtonMsg(KeyButtonMsg);
sf::View getUXView();

#endif // GRAPHICS_H
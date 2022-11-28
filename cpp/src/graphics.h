#include <SFML/Graphics.hpp>
#if defined(__APPLE__)
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#else
#include <GL/gl.h>
#include <GL/glu.h>
#endif
#include <boost/shared_ptr.hpp>
#include "engine.h"
#include "input.h"
#include "interface.h"
#include "tutorial.h"

#ifndef GRAPHICS_H
#define GRAPHICS_H

const vector2fl COMBATUNIT_SHOT_OFFSET(20, 10);

sf::RenderWindow* setupGraphics(bool fullscreen, bool smallScreen);
void display(sf::RenderWindow *window, Game *game, GameUI* ui, optional<uint8_t> maybePlayerId, Tutorial*, sf::Font mainFont, sf::Font tutorialFont, bool drawWalletHints);
void loadFonts(sf::Font* mainFont, sf::Font* tutorialFont);
void cleanupGraphics(sf::RenderWindow* window);

#endif // GRAPHICS_H
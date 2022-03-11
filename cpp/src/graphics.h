#include <SFML/Graphics.hpp>
#include <GL/gl.h>
#include <GL/glu.h>
#include <boost/shared_ptr.hpp>
#include "engine.h"
#include "input.h"

#ifndef GRAPHICS_H
#define GRAPHICS_H

sf::RenderWindow setupGraphics();
void display(sf::RenderWindow *window, Game *game, UI ui, int playerId);
void cleanupGraphics();

#endif // GRAPHICS_H
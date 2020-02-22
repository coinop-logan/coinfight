#include <SFML/Graphics.hpp>
#include <GL/gl.h>
#include <GL/glu.h>
#include <boost/shared_ptr.hpp>
#include "engine.h"

#ifndef GRAPHICS_H
#define GRAPHICS_H

void drawEntity(sf::RenderWindow &window, boost::shared_ptr<Entity> entity);

#endif // GRAPHICS_H
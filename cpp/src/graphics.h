#include <SFML/Graphics.hpp>
#include <GL/gl.h>
#include <GL/glu.h>
#include <boost/shared_ptr.hpp>
#include "engine.h"
#include "input.h"

#ifndef GRAPHICS_H
#define GRAPHICS_H

struct Particle
{
    vector2f pos;
    vector2f velocity;
    Target target;
    sf::Color color;
    bool dead;
    Particle(vector2f pos, Target target, sf::Color color);
    void iterate(const Game &game);
    void draw(sf::RenderWindow *window, CameraState camera);
};
struct ParticlesContainer
{
    vector<boost::shared_ptr<Particle>> particles;
    void iterateParticles(const Game &game);
    void drawParticles(sf::RenderWindow *window, CameraState camera);
    void add(boost::shared_ptr<Particle> particle);
};

sf::RenderWindow* setupGraphics();
void display(sf::RenderWindow *window, Game *game, UI ui, ParticlesContainer *particles, int playerId);
void cleanupGraphics();

#endif // GRAPHICS_H
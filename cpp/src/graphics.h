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
struct LineParticle
{
    vector2f from, to;
    sf::Color color;
    int lifetime, timeLeft;
    bool dead;
    LineParticle(vector2f from, vector2f to, sf::Color color, int lifetime);
    void iterate();
    void draw(sf::RenderWindow *window, CameraState camera);
};
struct ParticlesContainer
{
    vector<boost::shared_ptr<Particle>> particles;
    vector<boost::shared_ptr<LineParticle>> lineParticles;
    void iterateParticles(const Game &game);
    void drawParticles(sf::RenderWindow *window, CameraState camera);
    void addParticle(boost::shared_ptr<Particle> particle);
    void addLineParticle(boost::shared_ptr<LineParticle> lineParticle);
};

sf::RenderWindow* setupGraphics(bool fullscreen);
void display(sf::RenderWindow *window, Game *game, UI ui, ParticlesContainer *particles, int playerId);
void cleanupGraphics();

#endif // GRAPHICS_H
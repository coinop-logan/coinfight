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

#ifndef GRAPHICS_H
#define GRAPHICS_H

const vector2fl COMBATUNIT_SHOT_OFFSET(20, 10);

// const int HEALTH_BAR_HP_PER_PIXEL = 10;

struct Particle
{
    vector2fl pos;
    vector2fl velocity;
    Target target;
    sf::Color color;
    bool dead;
    Particle(vector2fl pos, Target target, sf::Color color);
    void iterate(const Game &game);
    void drawWithColor(sf::RenderWindow *window, CameraState camera, sf::Color color);
    void draw(sf::RenderWindow *window, CameraState camera);
};
struct FadingParticle : public Particle
{
    vector2fl startPos;
    bool fadeOut;
    FadingParticle(vector2fl pos, Target target, sf::Color color, bool fadeOut);
    void draw(sf::RenderWindow *window, CameraState camera);
};
struct LineParticle
{
    vector2fl from, to;
    sf::Color color;
    float width;
    int lifetime, timeLeft;
    bool dead;
    LineParticle(vector2fl from, vector2fl to, sf::Color color, float width, int lifetime);
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

sf::RenderWindow* setupGraphics(bool fullscreen, bool smallScreen);
void display(sf::RenderWindow *window, Game *game, UI ui, ParticlesContainer *particles, optional<uint8_t> maybePlayerId);
void cleanupGraphics();

#endif // GRAPHICS_H
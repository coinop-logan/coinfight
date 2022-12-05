#include "myvectors.h"
#include "engine.h"
#include "interface.h"

#ifndef PARTICLES_H
#define PARTICLES_H

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

#endif // PARTICLES_H
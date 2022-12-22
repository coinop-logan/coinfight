#include "particles.h"
#include "graphics.h"

class sfLine : public sf::Drawable
{
private:
    sf::Vertex vertices[4];
public:
    sfLine(const sf::Vector2f& point1, const sf::Vector2f& point2, sf::Color color, float width)
    {
        sf::Vector2f direction = point2 - point1;
        sf::Vector2f unitDirection = direction/std::sqrt(direction.x*direction.x+direction.y*direction.y);
        sf::Vector2f unitPerpendicular(-unitDirection.y,unitDirection.x);

        sf::Vector2f offset = (width/2.f)*unitPerpendicular;

        vertices[0].position = point1 + offset;
        vertices[1].position = point2 + offset;
        vertices[2].position = point2 - offset;
        vertices[3].position = point1 - offset;

        for (int i=0; i<4; ++i)
            vertices[i].color = color;
    }

    void draw(sf::RenderTarget &target, const sf::RenderStates &states) const
    {
        target.draw(vertices,4,sf::PrimitiveType::TriangleFan);
    }
};

Particle::Particle(vector2fl pos, Target target, sf::Color color)
    : pos(pos), velocity(randomVectorWithMagnitude(3)), target(target), color(color), dead(false)
{}
FadingParticle::FadingParticle(vector2fl pos, Target target, sf::Color color, bool fadeOut)
    : Particle(pos, target, color), startPos(pos), fadeOut(fadeOut)
{}

void Particle::iterate(const Game &game)
{
    if (auto targetPos = target.getPointUnlessTargetDeleted(game))
    {
        vector2fl toTarget = vector2fl(*targetPos) - pos;
        if (toTarget.getMagnitude() < 10)
        {
            dead = true;
            return;
        }

        velocity += toTarget.normalized() * PARTICLE_MAGNET_STRENGTH;
        velocity *= PARTICLE_FRICTION_CONSTANT;
        pos += velocity;
    }
    else
        dead = true;

}
void Particle::drawWithColor(sf::RenderWindow *window, CameraState camera, sf::Color whichColor)
{
    sf::RectangleShape pixel(sf::Vector2f(2,2));
    pixel.setOrigin(sf::Vector2f(1,1));
    vector2fl drawPos = gamePosToScreenPos(camera, vector2fp(pos));
    pixel.setPosition(toSFVec(drawPos));
    pixel.setFillColor(whichColor);

    window->draw(pixel);
}
void Particle::draw(sf::RenderWindow *window, CameraState camera)
{
    drawWithColor(window, camera, this->color);
}
void FadingParticle::draw(sf::RenderWindow *window, CameraState camera)
{
    if (auto targetPoint = target.castToPoint())
    {
        float progressRatio = (pos - vector2fl(*targetPoint)).getMagnitudeSquared() / (startPos - vector2fl(*targetPoint)).getMagnitudeSquared();
        float alphaFloat = this->fadeOut ? (1-progressRatio) : progressRatio;
        int alphaInt = alphaFloat * 255;

        sf::Color fadedColor(this->color);
        fadedColor.a = alphaInt;
        drawWithColor(window, camera, fadedColor);
    }
    else
    {
        dead = true;
    }
}

LineParticle::LineParticle(vector2fl from, vector2fl to, sf::Color color, float width, int lifetime)
    : from(from), to(to), color(color), width(width), lifetime(lifetime), timeLeft(lifetime), dead(false)
{}
void LineParticle::iterate()
{
    timeLeft --;
    if (timeLeft <= 0)
        dead = true;
}
void LineParticle::draw(sf::RenderWindow *window, CameraState camera)
{
    vector2i drawFrom = gamePosToScreenPos(camera, vector2fp(from));
    vector2i drawTo = gamePosToScreenPos(camera, vector2fp(to));

    color.a = ((float)timeLeft / lifetime) * 255;

    sfLine line(sf::Vector2f(drawFrom.x, drawFrom.y), sf::Vector2f(drawTo.x, drawTo.y), color, width);

    window->draw(line);
}

void ParticlesContainer::iterateParticles(const Game &game)
{
    for (unsigned int i=0; i<particles.size(); i++)
    {
        if (!particles[i]->dead)
            particles[i]->iterate(game);
        else
        {
            particles.erase(particles.begin()+i);
            i--;
        }
    }
    for (unsigned int i=0; i<lineParticles.size(); i++)
    {
        if (!lineParticles[i]->dead)
            lineParticles[i]->iterate();
        else
        {
            lineParticles.erase(lineParticles.begin()+i);
            i--;
        }
    }
}
void ParticlesContainer::drawParticles(sf::RenderWindow *window, CameraState camera)
{
    for (unsigned int i=0; i<particles.size(); i++)
    {
        particles[i]->draw(window, camera);
    }
    for (unsigned int i=0; i<lineParticles.size(); i++)
    {
        lineParticles[i]->draw(window, camera);
    }
}
void ParticlesContainer::addParticle(boost::shared_ptr<Particle> particle)
{
    particles.push_back(particle);
}
void ParticlesContainer::addLineParticle(boost::shared_ptr<LineParticle> lineParticle)
{
    lineParticles.push_back(lineParticle);
}
#include <boost/shared_ptr.hpp>
#include <iostream>
#include <fstream>
#include <SFML/Graphics.hpp>
#include "engine.h"
#include "graphics.h"
#include "common.h"
#include "config.h"

using namespace std;

sf::Font mainFont;

sf::RenderWindow* setupGraphics()
{
    if (!mainFont.loadFromFile("Andale_Mono.ttf"))
        throw runtime_error("Can't load font");

    sf::RenderWindow *window = new sf::RenderWindow(sf::VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), "Coinfight Client", sf::Style::Close | sf::Style::Titlebar);

    // const uint8_t pixels[] =
    //     {255, 0, 0, 255,   0, 0, 0, 0,   0, 0, 0, 0,   0, 0, 0, 0,
    //     0, 0, 0, 0,   255, 0, 0, 255,   0, 0, 0, 0,   0, 0, 0, 0,
    //     0, 0, 0, 0,   0, 0, 0, 0,   255, 0, 0, 255,   0, 0, 0, 0,
    //     0, 0, 0, 0,   0, 0, 0, 0,   0, 0, 0, 0,   255, 0, 0, 255
    //     };
    // sf::Vector2u size(4, 4);
    // sf::Vector2u hotspot(0,0);

    // sf::Cursor cursor;
    // if (cursor.loadFromPixels(
    //     pixels,
    //     size,
    //     hotspot
    // ))
    //     window->setMouseCursor(cursor);
    
    return window;
}

void drawEntity(sf::RenderWindow *window, boost::shared_ptr<Entity> entity, CameraState camera)
{
    vector2i drawPos = gamePosToScreenPos(camera, vector2i(entity->pos));
    sf::Color primaryColor = entity->getPrimaryColor();
    float drawRotation = -entity->getRotation();

    if (boost::shared_ptr<GoldPile> goldPile = boost::dynamic_pointer_cast<GoldPile, Entity>(entity))
    {
        int size = ceil(sqrt(goldPile->gold.getInt() / 30.0)) + 1;
        if (size > 1)
        {
            sf::CircleShape triangle(size, 3);
            triangle.setOrigin(triangle.getRadius(), triangle.getRadius());
            triangle.setFillColor(primaryColor);
            triangle.setPosition(drawPos.x, drawPos.y);
            window->draw(triangle);
        }
    }
    else if (boost::shared_ptr<Prime> castedEntity = boost::dynamic_pointer_cast<Prime, Entity>(entity))
    {
        sf::ConvexShape oneSide;
        oneSide.setPointCount(3);

        oneSide.setFillColor(primaryColor);
        oneSide.setPosition(drawPos.x, drawPos.y);
        oneSide.setRotation(radToDeg(drawRotation));

        // draw two triangles
        oneSide.setPoint(1, sf::Vector2f(12, 0));
        oneSide.setPoint(0, sf::Vector2f(-4, 0));
        oneSide.setPoint(2, sf::Vector2f(-12, 8));
        window->draw(oneSide);
        oneSide.setPoint(2, sf::Vector2f(-12, -8));
        window->draw(oneSide);
    }
    else if (boost::shared_ptr<Gateway> castedEntity = boost::dynamic_pointer_cast<Gateway, Entity>(entity))
    {
        sf::RectangleShape rect(sf::Vector2f(20, 20));
        rect.setOrigin(10, 10);
        rect.setFillColor(primaryColor);
        rect.setPosition(drawPos.x, drawPos.y);
        window->draw(rect);
    }
    else
    {
        throw runtime_error("No drawing code implemented for " + entity->getTypeName() + ".");
    }
}

void drawOutputStrings(sf::RenderWindow *window, vector<sf::String> strings)
{
    for (uint i=0; i<strings.size(); i++)
    {
        sf::Text text(strings[i], mainFont, 16);
        text.setFillColor(sf::Color(150, 150, 150));

        float width = text.getLocalBounds().width;
        int x, y;
        x = window->getSize().x - width - 10;
        y = 6 + i * 20;
        text.setPosition(sf::Vector2f(x, y));

        window->draw(text);
    }
}

Particle::Particle(vector2f pos, Target target, sf::Color color)
    : pos(pos), target(target), velocity(velocity), color(color), dead(false)
{
    velocity = randomVectorWithMagnitude(3);
}

void Particle::iterate(const Game &game)
{
    if (auto targetPos = target.getPointUnlessTargetDeleted(game))
    {
        vector2f toTarget = *targetPos - pos;
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
void Particle::draw(sf::RenderWindow *window, CameraState camera)
{
    sf::RectangleShape pixel(sf::Vector2f(1,1));
    vector2i drawPos = gamePosToScreenPos(camera, pos);
    pixel.setPosition(drawPos.x, drawPos.y);
    pixel.setFillColor(color);

    window->draw(pixel);
}

void ParticlesContainer::iterateParticles(const Game &game)
{
    for (uint i=0; i<particles.size(); i++)
    {
        if (!particles[i]->dead)
            particles[i]->iterate(game);
        else
        {
            particles.erase(particles.begin()+i);
            i--;
        }
    }
}
void ParticlesContainer::drawParticles(sf::RenderWindow *window, CameraState camera)
{
    for (uint i=0; i<particles.size(); i++)
    {
        particles[i]->draw(window, camera);
    }
}
void ParticlesContainer::add(boost::shared_ptr<Particle> particle)
{
    particles.push_back(particle);
}

void drawCircleAround(sf::RenderWindow *window, vector2i screenPos, uint radius, uint thickness, sf::Color color)
{
    sf::Transform mouseTransform;
    mouseTransform.translate(screenPos.x, screenPos.y);

    sf::CircleShape circle(radius);
    circle.setOrigin(radius, radius);
    circle.setOutlineColor(color);
    circle.setFillColor(sf::Color::Transparent);
    circle.setOutlineThickness(thickness);

    window->draw(circle, mouseTransform);
}

void drawNormalCursor(sf::RenderWindow *window)
{
    window->setMouseCursorVisible(true);
}

void drawTargetCursor(sf::RenderWindow *window, vector2i mousePos, sf::Color color)
{
    window->setMouseCursorVisible(false);
    
    sf::VertexArray lines(sf::Lines, 2);
    lines[0].position = sf::Vector2f(CURSOR_SIZE/2, CURSOR_SIZE/2);
    lines[1].position = sf::Vector2f(4, 4);
    lines[0].color = color;
    lines[1].color = color;

    sf::Transform transform;
    transform.translate(mousePos.x, mousePos.y);
    window->draw(lines, transform);
    transform.rotate(90);
    window->draw(lines, transform);
    transform.rotate(90);
    window->draw(lines, transform);
    transform.rotate(90);
    window->draw(lines, transform);

    sf::RectangleShape rect(sf::Vector2f(2,2));
    rect.setPosition(-1, -1);
    rect.setFillColor(color);
    
    transform = sf::Transform();
    transform.translate(mousePos.x, mousePos.y);
    window->draw(rect, transform);
}
void drawBracketsCursor(sf::RenderWindow *window, vector2i mousePos, sf::Color color)
{
    window->setMouseCursorVisible(false);
    
    sf::VertexArray lines(sf::LinesStrip, 3);
    lines[0].position = sf::Vector2f(CURSOR_SIZE/2 - 6, CURSOR_SIZE/2);
    lines[1].position = sf::Vector2f(CURSOR_SIZE/2, CURSOR_SIZE/2);
    lines[2].position = sf::Vector2f(CURSOR_SIZE/2, CURSOR_SIZE/2 - 6);
    lines[0].color = color;
    lines[1].color = color;
    lines[2].color = color;

    sf::Transform transform;
    transform.translate(mousePos.x, mousePos.y);
    window->draw(lines, transform);
    transform.rotate(90);
    window->draw(lines, transform);
    transform.rotate(90);
    window->draw(lines, transform);
    transform.rotate(90);
    window->draw(lines, transform);
}

void drawSelectableCursor(sf::RenderWindow *window, vector2i mousePos)
{
    drawBracketsCursor(window, mousePos, sf::Color::Green);
}

void drawCursor(sf::RenderWindow *window, UI ui)
{
    vector2i mousePos = vector2i(sf::Mouse::getPosition(*window).x, sf::Mouse::getPosition(*window).y);

    // depending on cmdState and mouseoverEntity
    switch (ui.cmdState)
    {
        case UI::Default:
            if (ui.mouseoverEntity)
            {
                drawSelectableCursor(window, mousePos);
            }
            else
            {
                drawNormalCursor(window);
            }
            break;
        case UI::Deposit:
            if (ui.mouseoverEntity)
            {
                drawBracketsCursor(window, mousePos, sf::Color::Blue);
            }
            else
            {
                drawTargetCursor(window, mousePos, sf::Color::Blue);
            }
            break;
    }
}

void drawSelectionCircleAroundEntity(sf::RenderWindow *window, CameraState camera, boost::shared_ptr<Entity> entity)
{
    drawCircleAround(window, gamePosToScreenPos(camera, entity->pos), 15, 1, sf::Color::Green);
}

void display(sf::RenderWindow *window, Game *game, UI ui, ParticlesContainer *particles, int playerIdOrNegativeOne)
{
    window->clear();

    particles->drawParticles(window, ui.camera);
    particles->iterateParticles(*game);

    for (unsigned int i = 0; i < game->entities.size(); i++)
    {
        if (game->entities[i])
        {
            drawEntity(window, game->entities[i], ui.camera);

            // and add some particles every now and then
            if (game->frame % 3 == 0)
            {
                if (auto prime = boost::dynamic_pointer_cast<Prime, Entity>(game->entities[i]))
                {
                    if (prime->goldTransferState == Prime::Pulling)
                    {
                        if (optional<vector2f> maybeTargetPos = prime->getTarget().getPointUnlessTargetDeleted(*game))
                        {
                            vector2f targetPos = *maybeTargetPos;
                            particles->add(boost::shared_ptr<Particle>(new Particle(targetPos, Target(prime->ref), sf::Color::Yellow)));
                        }
                    }
                    else if (prime->goldTransferState == Prime::Pushing)
                    {
                        particles->add(boost::shared_ptr<Particle>(new Particle(prime->pos, prime->getTarget(), sf::Color::Yellow)));
                    }
                }
                if (auto gateway = boost::dynamic_pointer_cast<Gateway, Entity>(game->entities[i]))
                {
                    if (gateway->maybeBuildingUnit)
                    {
                        particles->add(boost::shared_ptr<Particle>(new Particle(gateway->pos, Target(gateway->maybeBuildingUnit->ref), sf::Color::Yellow)));
                    }
                }
            }
        }
    }
    for (uint i=0; i<ui.selectedEntities.size(); i++)
    {
        drawSelectionCircleAroundEntity(window, ui.camera, ui.selectedEntities[i]);
    }

    vector<sf::String> outputStrings;

    if (playerIdOrNegativeOne >= 0)
    {
        uint playerId = playerIdOrNegativeOne;
        outputStrings.push_back(game->players[playerId].credit.getDollarString());
    }

    for (uint i=0; i<game->entities.size(); i++)
    {
        if (auto prime = boost::dynamic_pointer_cast<Prime, Entity>(game->entities[i]))
        {
            outputStrings.push_back("prime: " + prime->heldGold.getDollarString());
        }
        if (auto goldpile = boost::dynamic_pointer_cast<GoldPile, Entity>(game->entities[i]))
        {
            outputStrings.push_back("goldpile: " + goldpile->gold.getDollarString());
        }
    }

    drawOutputStrings(window, outputStrings);

    drawCursor(window, ui);
    
    window->display();
}
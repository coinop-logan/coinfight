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
    sf::Color primaryColor = entity->getPrimaryColor(); // may be modified later if unit is not yet active
    sf::Color outlineColor(primaryColor);
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
    else if (auto unit = boost::dynamic_pointer_cast<Unit, Entity>(entity))
    {
        // make transparent if not active
        if (!unit->isActive())
        {
            float newAlpha = unit->getBuiltRatio() * 0.8 * 255;
            primaryColor.a = newAlpha;
        }

        if (auto prime = boost::dynamic_pointer_cast<Prime, Unit>(unit))
        {
            sf::ConvexShape oneSide;
            oneSide.setPointCount(3);

            oneSide.setFillColor(primaryColor);
            oneSide.setPosition(drawPos.x, drawPos.y);
            oneSide.setRotation(radToDeg(drawRotation));

            sf::Vector2f front = sf::Vector2f(12, 0);
            sf::Vector2f back = sf::Vector2f(-4, 0);
            sf::Vector2f right = sf::Vector2f(-12, 8);
            sf::Vector2f left = sf::Vector2f(-12, -8);

            // draw two triangles
            oneSide.setPoint(1, front);
            oneSide.setPoint(0, back);
            oneSide.setPoint(2, right);
            window->draw(oneSide);
            oneSide.setPoint(2, left);
            window->draw(oneSide);

            // draw outline
            sf::VertexArray lines(sf::LinesStrip, 5);
            lines[0].position = front;
            lines[1].position = right;
            lines[2].position = back;
            lines[3].position = left;
            lines[4].position = front;
            lines[0].color = outlineColor;
            lines[1].color = outlineColor;
            lines[2].color = outlineColor;
            lines[3].color = outlineColor;
            lines[4].color = outlineColor;
            sf::Transform transform;
            transform.translate(drawPos.x, drawPos.y);
            transform.rotate(radToDeg(drawRotation));
            window->draw(lines, transform);
        }
        else if (auto fighter = boost::dynamic_pointer_cast<Fighter, Unit>(unit))
        {
            sf::ConvexShape oneSide;
            oneSide.setPointCount(3);

            oneSide.setFillColor(primaryColor);
            oneSide.setPosition(drawPos.x, drawPos.y);
            oneSide.setRotation(radToDeg(drawRotation));

            sf::Vector2f front = sf::Vector2f(12, 0);
            sf::Vector2f back = sf::Vector2f(-4, 0);
            sf::Vector2f right = sf::Vector2f(-12, 16);
            sf::Vector2f left = sf::Vector2f(-12, -16);

            // draw two triangles
            oneSide.setPoint(1, front);
            oneSide.setPoint(0, back);
            oneSide.setPoint(2, right);
            window->draw(oneSide);
            oneSide.setPoint(2, left);
            window->draw(oneSide);

            // draw outline
            sf::VertexArray lines(sf::LinesStrip, 5);
            lines[0].position = front;
            lines[1].position = right;
            lines[2].position = back;
            lines[3].position = left;
            lines[4].position = front;
            lines[0].color = outlineColor;
            lines[1].color = outlineColor;
            lines[2].color = outlineColor;
            lines[3].color = outlineColor;
            lines[4].color = outlineColor;
            sf::Transform transform;
            transform.translate(drawPos.x, drawPos.y);
            transform.rotate(radToDeg(drawRotation));
            window->draw(lines, transform);
        }
        else if (auto gateway = boost::dynamic_pointer_cast<Gateway, Unit>(unit))
        {
            sf::RectangleShape rect(sf::Vector2f(20, 20));
            rect.setOrigin(10, 10);
            rect.setFillColor(primaryColor);
            rect.setOutlineColor(outlineColor);
            rect.setOutlineThickness(1);
            rect.setPosition(drawPos.x, drawPos.y);
            window->draw(rect);
        }
        else
        {
            throw runtime_error("No drawing code implemented for " + entity->getTypeName() + ".");
        }
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
    : pos(pos), velocity(randomVectorWithMagnitude(3)), target(target), color(color), dead(false)
{}

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

LineParticle::LineParticle(vector2f from, vector2f to, sf::Color color, int lifetime)
    : from(from), to(to), color(color), lifetime(lifetime), timeLeft(lifetime), dead(false)
{}
void LineParticle::iterate()
{
    cout << timeLeft -- << endl;
    if (timeLeft <= 0)
        dead = true;
}
void LineParticle::draw(sf::RenderWindow *window, CameraState camera)
{
    sf::VertexArray lines(sf::Lines, 2);
    vector2i drawFrom = gamePosToScreenPos(camera, from);
    vector2i drawTo = gamePosToScreenPos(camera, to);
    lines[0].position = sf::Vector2f(drawFrom.x, drawFrom.y);
    lines[1].position = sf::Vector2f(drawTo.x, drawTo.y);

    color.a = ((float)timeLeft / lifetime) * 255;
    lines[0].color = color;
    lines[1].color = color;

    window->draw(lines);
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
    for (uint i=0; i<lineParticles.size(); i++)
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
    for (uint i=0; i<particles.size(); i++)
    {
        particles[i]->draw(window, camera);
    }
    for (uint i=0; i<lineParticles.size(); i++)
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

void drawGhostBuilding(sf::RenderWindow *window, const UI &ui, vector2f mousePos)
{
    window->setMouseCursorVisible(false);
    
    ui.ghostBuilding->pos = screenPosToGamePos(ui.camera, mousePos);
    drawEntity(window, ui.ghostBuilding, ui.camera);
}

void drawCursor(sf::RenderWindow *window, UI ui, int playerId)
{
    vector2i mousePos = vector2i(sf::Mouse::getPosition(*window).x, sf::Mouse::getPosition(*window).y);

    // depending on cmdState and mouseoverEntity
    switch (ui.cmdState)
    {
        case UI::Default:
            if (ui.mouseoverEntity)
            {
                if (getAllianceType(playerId, ui.mouseoverEntity) == Enemy)
                {
                    drawBracketsCursor(window, mousePos, sf::Color::Red);
                }
                else
                {
                    drawSelectableCursor(window, mousePos);
                }
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
        case UI::Build:
            drawGhostBuilding(window, ui, mousePos);
            break;
    }
}

void drawSelectionCircleAroundEntity(sf::RenderWindow *window, CameraState camera, boost::shared_ptr<Entity> entity)
{
    drawCircleAround(window, gamePosToScreenPos(camera, entity->pos), 15, 1, sf::Color::Green);
}

void drawUnitDroppableValues(sf::RenderWindow *window, Game *game, UI ui, int playerIdOrNegativeOne)
{
    for (uint i=0; i<game->entities.size(); i++)
    {
        if (!game->entities[i])
            continue;
        Coins *displayAboveCoins = NULL;
        Coins *displayBelowCoins = NULL;

        sf::Color topTextColor;
        switch (getAllianceType(playerIdOrNegativeOne, game->entities[i]))
        {
            case Ally:
                topTextColor = sf::Color::Green;
                break;
            case Enemy:
                topTextColor = sf::Color::Red;
                break;
            case Neutral:
                topTextColor = sf::Color::Yellow;
                break;
        }

        if (auto goldpile = boost::dynamic_pointer_cast<GoldPile, Entity>(game->entities[i]))
        {
            displayAboveCoins = &goldpile->gold;
        }
        else if (auto unit = boost::dynamic_pointer_cast<Unit, Entity>(game->entities[i]))
        {
            displayAboveCoins = &unit->goldInvested;
            if (auto prime = boost::dynamic_pointer_cast<Prime, Unit>(unit))
            {
                displayBelowCoins = &prime->heldGold;
            }
        }

        vector2f entityPos = game->entities[i]->pos;
        if (displayAboveCoins)
        {
            sf::Text aboveText(displayAboveCoins->getDollarString(), mainFont, 16);
            sf::FloatRect textRec = aboveText.getLocalBounds();

            vector2f textGamePos = entityPos + vector2f(0, 30);
            vector2f textScreenPos = gamePosToScreenPos(ui.camera, textGamePos);

            aboveText.setFillColor(topTextColor);
            aboveText.setOrigin(textRec.width / 2, textRec.height / 2);
            aboveText.setPosition(textScreenPos.x, textScreenPos.y);

            sf::RectangleShape drawRect(sf::Vector2f(textRec.width + 3, textRec.height + 3));
            drawRect.setOrigin(textRec.width / 2, textRec.height / 2);
            drawRect.setPosition(textScreenPos.x, textScreenPos.y + 3);
            drawRect.setFillColor(sf::Color(0, 0, 0, 150));

            window->draw(drawRect);
            window->draw(aboveText);
        }
        if (displayBelowCoins && displayBelowCoins->getInt() > 0)
        {
            sf::Text belowText(displayBelowCoins->getDollarString(), mainFont, 16);
            sf::FloatRect textRec = belowText.getLocalBounds();

            vector2f textGamePos = entityPos + vector2f(0, -20);
            vector2f textScreenPos = gamePosToScreenPos(ui.camera, textGamePos);

            belowText.setFillColor(sf::Color(200, 200, 255));
            belowText.setOrigin(textRec.width / 2, textRec.height / 2);
            belowText.setPosition(textScreenPos.x, textScreenPos.y);

            sf::RectangleShape drawRect(sf::Vector2f(textRec.width + 3, textRec.height + 3));
            drawRect.setOrigin(textRec.width / 2, textRec.height / 2);
            drawRect.setPosition(textScreenPos.x, textScreenPos.y + 3);
            drawRect.setFillColor(sf::Color(0, 0, 0, 150));

            window->draw(drawRect);
            window->draw(belowText);
        }
    }
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

            // add some gold particles every now and then
            if (game->frame % 3 == 0)
            {
                if (auto prime = boost::dynamic_pointer_cast<Prime, Entity>(game->entities[i]))
                {
                    if (prime->goldTransferState == Prime::Pulling)
                    {
                        if (optional<vector2f> maybeTargetPos = prime->getTarget().getPointUnlessTargetDeleted(*game))
                        {
                            vector2f targetPos = *maybeTargetPos;
                            particles->addParticle(boost::shared_ptr<Particle>(new Particle(targetPos, Target(prime->ref), sf::Color::Yellow)));
                        }
                    }
                    else if (prime->goldTransferState == Prime::Pushing)
                    {
                        particles->addParticle(boost::shared_ptr<Particle>(new Particle(prime->pos, prime->getTarget(), sf::Color::Yellow)));
                    }
                }
                if (auto gateway = boost::dynamic_pointer_cast<Gateway, Entity>(game->entities[i]))
                {
                    if (gateway->maybeBuildingUnit)
                    {
                        particles->addParticle(boost::shared_ptr<Particle>(new Particle(gateway->pos, Target(gateway->maybeBuildingUnit->ref), sf::Color::Yellow)));
                    }
                }
            }

            // fighter shots
            if (auto fighter = boost::dynamic_pointer_cast<Fighter, Entity>(game->entities[i]))
            {
                if (fighter->animateShot != Fighter::None)
                {
                    if (optional<vector2f> targetPos = fighter->getTarget().getPointUnlessTargetDeleted(*game))
                    {
                        vector2f relativeShotStartPos;
                        if (fighter->animateShot == Fighter::Left)
                        {
                            relativeShotStartPos = vector2f(0, -10);
                        }
                        else
                        {
                            relativeShotStartPos = vector2f(0, 10);
                        }
                        vector2f rotated = relativeShotStartPos.rotated(fighter->angle_view);
                        vector2f final = fighter->pos + rotated;
                        boost::shared_ptr<LineParticle> line(new LineParticle(final, *targetPos, sf::Color::Red, 8));
                        particles->addLineParticle(line);
                    }
                }
            }
        }
    }
    for (uint i=0; i<ui.selectedEntities.size(); i++)
    {
        drawSelectionCircleAroundEntity(window, ui.camera, ui.selectedEntities[i]);
    }

    drawUnitDroppableValues(window, game, ui, playerIdOrNegativeOne);

    vector<sf::String> outputStrings;

    if (playerIdOrNegativeOne >= 0)
    {
        uint playerId = playerIdOrNegativeOne;
        outputStrings.push_back("Account balance: " + game->players[playerId].credit.getDollarString());
    }

    // for (uint i=0; i<game->entities.size(); i++)
    // {
    //     if (auto prime = boost::dynamic_pointer_cast<Prime, Entity>(game->entities[i]))
    //     {
    //         outputStrings.push_back("prime: " + prime->heldGold.getDollarString());
    //     }
    //     if (auto goldpile = boost::dynamic_pointer_cast<GoldPile, Entity>(game->entities[i]))
    //     {
    //         outputStrings.push_back("goldpile: " + goldpile->gold.getDollarString());
    //     }
    // }

    drawOutputStrings(window, outputStrings);

    drawCursor(window, ui, playerIdOrNegativeOne);
    
    window->display();
}
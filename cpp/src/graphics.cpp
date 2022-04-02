#include <boost/shared_ptr.hpp>
#include <iostream>
#include <fstream>
#include <SFML/Graphics.hpp>
#include "engine.h"
#include "graphics.h"
#include "common.h"
#include "config.h"

using namespace std;

const sf::Color GATEWAY_MAIN_COLOR = sf::Color(100,100,255);
const sf::Color GATEWAY_INNEROUTLINE_COLOR = sf::Color(0,0,255);

const sf::Color FIGHTER_BARREL_COLOR = sf::Color::Red;

sf::Font mainFont;

sf::RenderWindow* setupGraphics(bool fullscreen)
{
    if (!mainFont.loadFromFile("Andale_Mono.ttf"))
        throw runtime_error("Can't load font");

    auto flags = fullscreen ? sf::Style::Close | sf::Style::Fullscreen : sf::Style::Close | sf::Style::Titlebar ;
    sf::RenderWindow *window = new sf::RenderWindow(sf::VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), "Coinfight Client", flags);
    window->setKeyRepeatEnabled(false);
    
    return window;
}

void drawBackground(sf::RenderWindow *window, CameraState camera)
{
    sf::Color backgroundColor(0, 0, 50);
    window->clear(backgroundColor);

    sf::Color backgroundCirclesColor(60, 0, 150);
    sf::Color backgroundSpokeEndColor(backgroundCirclesColor);
    backgroundSpokeEndColor.a = 50;
    vector2f centerOfMapScreenPos = gamePosToScreenPos(camera, vector2f(0,0));

    sf::CircleShape circle(1, 40);
    circle.setFillColor(sf::Color::Transparent);
    circle.setOutlineColor(backgroundCirclesColor);
    circle.setOutlineThickness(1);
    circle.setPosition(centerOfMapScreenPos.x, centerOfMapScreenPos.y);

    for (uint i=0; i<10; i++)
    {
        float radius = 10 * pow(2, i+1);
        float nextRadius = 10 * pow(2, i+2);

        // draw circle
        circle.setRadius(radius);
        circle.setPointCount(i * 20);
        circle.setOrigin(radius, radius);

        window->draw(circle);

        if (i > 0)
        {
            uint numSpokes = pow(2, i);

            sf::VertexArray lines(sf::Lines, numSpokes*2);
            for (uint i=0; i<numSpokes; i++)
            {
                float angle = ((float)i / numSpokes) * 2 * M_PI;
                vector2f from = gamePosToScreenPos(camera, composeVector2f(angle, radius));
                vector2f to = gamePosToScreenPos(camera, composeVector2f(angle, nextRadius));

                lines[i*2].position = sf::Vector2f(from.x, from.y);
                lines[i*2+1].position = sf::Vector2f(to.x, to.y);
                lines[i*2].color = backgroundCirclesColor;
                lines[i*2+1].color = backgroundSpokeEndColor;

                sf::Transform transform;
            }

            window->draw(lines);
        }
    }
}

void drawGoldPile(sf::RenderWindow *window, boost::shared_ptr<GoldPile> goldPile, vector2f drawPos)
{
    float width = ceil(sqrt(goldPile->gold.getInt() / 30.0)) + 1;
    float height = width * .4;
    if (width > 1)
    {
        sf::Color goldBottomColor(sf::Color(255, 180, 0));
        sf::Color goldTopColor(sf::Color::Yellow);

        sf::VertexArray diamond(sf::Quads, 4);
        diamond[0].position = sf::Vector2f(width/2, 0);
        diamond[1].position = sf::Vector2f(width/3, -height);
        diamond[2].position = sf::Vector2f(-width/3, -height);
        diamond[3].position = sf::Vector2f(-width/2, 0);
        diamond[0].color = goldBottomColor;
        diamond[1].color = goldTopColor;
        diamond[2].color = goldTopColor;
        diamond[3].color = goldBottomColor;

        sf::Transform transform;
        transform.translate(drawPos.x, drawPos.y + height/2);

        window->draw(diamond, transform);
    }
}

const sf::Color unitOutlineColor(100, 100, 100);

void drawBeacon(sf::RenderWindow *window, vector2f drawPos, sf::Color teamColor, uint alpha)
{
    sf::Color teamColorFaded(teamColor.r, teamColor.g, teamColor.r, alpha);

    sf::Color fadedInnerOutlineColor(GATEWAY_INNEROUTLINE_COLOR.r, GATEWAY_INNEROUTLINE_COLOR.g, GATEWAY_INNEROUTLINE_COLOR.b, alpha);
    sf::RectangleShape innerRect(sf::Vector2f(10, 10));
    innerRect.setOrigin(5, 5);
    innerRect.setFillColor(teamColorFaded);
    innerRect.setOutlineColor(teamColor);
    innerRect.setOutlineThickness(1.5);
    innerRect.setRotation(45);
    innerRect.setPosition(drawPos.x, drawPos.y);

    window->draw(innerRect);
}

void drawGateway(sf::RenderWindow *window, vector2f drawPos, sf::Color teamColor, uint alpha)
{
    sf::Color fillColorFaded(GATEWAY_MAIN_COLOR.r, GATEWAY_MAIN_COLOR.g, GATEWAY_MAIN_COLOR.r, alpha);

    sf::CircleShape outerHex(15, 6);
    outerHex.setOrigin(15, 15);
    outerHex.setFillColor(fillColorFaded);
    outerHex.setOutlineColor(unitOutlineColor);
    outerHex.setOutlineThickness(1);
    outerHex.setPosition(drawPos.x, drawPos.y);
    outerHex.setRotation(90);

    window->draw(outerHex);

    drawBeacon(window, drawPos, teamColor, alpha);
}

void drawPrime(sf::RenderWindow *window, vector2f drawPos, float rotation, sf::Color teamColor, uint alpha)
{
    sf::Color fillColorFaded(teamColor.r, teamColor.g, teamColor.r, alpha);

    sf::ConvexShape oneSide;
    oneSide.setPointCount(3);

    oneSide.setFillColor(fillColorFaded);
    oneSide.setPosition(drawPos.x, drawPos.y);
    oneSide.setRotation(radToDeg(rotation));

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
    lines[0].color = unitOutlineColor;
    lines[1].color = unitOutlineColor;
    lines[2].color = unitOutlineColor;
    lines[3].color = unitOutlineColor;
    lines[4].color = unitOutlineColor;
    sf::Transform transform;
    transform.translate(drawPos.x, drawPos.y);
    transform.rotate(radToDeg(rotation));
    window->draw(lines, transform);
}

void drawFighter(sf::RenderWindow *window, vector2f drawPos, float rotation, sf::Color teamColor, uint alpha)
{
    sf::Color fillColorFaded(teamColor.r, teamColor.g, teamColor.r, alpha);

    sf::ConvexShape oneSide;
    oneSide.setPointCount(3);

    oneSide.setFillColor(fillColorFaded);
    oneSide.setPosition(drawPos.x, drawPos.y);
    oneSide.setRotation(radToDeg(rotation));

    sf::Vector2f front = sf::Vector2f(12, 0);
    sf::Vector2f back = sf::Vector2f(-4, 0);
    sf::Vector2f right = sf::Vector2f(-12, 16);
    sf::Vector2f left = sf::Vector2f(-12, -16);

    // draw gun barrels
    sf::Color fadedBarrelColor(FIGHTER_BARREL_COLOR.r, FIGHTER_BARREL_COLOR.g, FIGHTER_BARREL_COLOR.b, alpha);
    sf::VertexArray gunBarrelPoints(sf::Triangles, 3);
    gunBarrelPoints[0].position = sf::Vector2f(-8, 8);
    gunBarrelPoints[1].position = sf::Vector2f(-12, 16);
    gunBarrelPoints[2].position = sf::Vector2f(FIGHTER_SHOT_OFFSET.x, FIGHTER_SHOT_OFFSET.y);
    gunBarrelPoints[0].color = fadedBarrelColor;
    gunBarrelPoints[1].color = fadedBarrelColor;
    gunBarrelPoints[2].color = fadedBarrelColor;

    sf::Transform transform = sf::Transform();
    transform.translate(drawPos.x, drawPos.y);
    transform.rotate(radToDeg(rotation));
    window->draw(gunBarrelPoints, transform);

    gunBarrelPoints[0].position.y *= -1;
    gunBarrelPoints[1].position.y *= -1;
    gunBarrelPoints[2].position.y *= -1;
    window->draw(gunBarrelPoints, transform);

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
    lines[0].color = unitOutlineColor;
    lines[1].color = unitOutlineColor;
    lines[2].color = unitOutlineColor;
    lines[3].color = unitOutlineColor;
    lines[4].color = unitOutlineColor;

    transform = sf::Transform();
    transform.translate(drawPos.x, drawPos.y);
    transform.rotate(radToDeg(rotation));
    window->draw(lines, transform);
}

void drawUnit(sf::RenderWindow *window, boost::shared_ptr<Unit> unit, vector2f drawPos)
{
    sf::Color teamColor = unit->getTeamColor(); // may be modified later if unit is not yet active
    float drawRotation = -unit->getRotation();

    uint fadedAlpha;
    if (!unit->isActive())
    {
        fadedAlpha = unit->getBuiltRatio() * 0.8 * 255;
    }
    else
    {
        fadedAlpha = 255;
    }
    sf::Color teamColorFaded(teamColor.r, teamColor.g, teamColor.b, fadedAlpha);

    if (auto beacon = boost::dynamic_pointer_cast<Beacon, Unit>(unit))
    {
        drawBeacon(window, drawPos, teamColor, fadedAlpha);
    }
    else if (auto gateway = boost::dynamic_pointer_cast<Gateway, Unit>(unit))
    {
        drawGateway(window, drawPos, teamColor, fadedAlpha);
    }
    else if (auto prime = boost::dynamic_pointer_cast<Prime, Unit>(unit))
    {
        drawPrime(window, drawPos, drawRotation, teamColor, fadedAlpha);
    }
    else if (auto fighter = boost::dynamic_pointer_cast<Fighter, Unit>(unit))
    {
        drawFighter(window, drawPos, drawRotation, teamColor, fadedAlpha);
    }
    else {
        throw runtime_error("No drawing code implemented for unit " + unit->getTypeName() + ".");
    }
}

void drawEntity(sf::RenderWindow *window, boost::shared_ptr<Entity> entity, CameraState camera)
{
    vector2i drawPos = gamePosToScreenPos(camera, vector2i(entity->pos));

    if (boost::shared_ptr<GoldPile> goldPile = boost::dynamic_pointer_cast<GoldPile, Entity>(entity))
    {
        drawGoldPile(window, goldPile, drawPos);
    }
    else if (auto unit = boost::dynamic_pointer_cast<Unit, Entity>(entity))
    {
        drawUnit(window, unit, drawPos);
    }
    else
    {
        throw runtime_error("No drawing code implemented for entity " + entity->getTypeName() + ".");
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
FadingParticle::FadingParticle(vector2f pos, Target target, sf::Color color, bool fadeOut)
    : Particle(pos, target, color), startPos(pos), fadeOut(fadeOut)
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
void Particle::drawWithColor(sf::RenderWindow *window, CameraState camera, sf::Color whichColor)
{
    sf::RectangleShape pixel(sf::Vector2f(1,1));
    vector2i drawPos = gamePosToScreenPos(camera, pos);
    pixel.setPosition(drawPos.x, drawPos.y);
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
        float progressRatio = (pos - *targetPoint).getMagnitudeSquared() / (startPos - *targetPoint).getMagnitudeSquared();
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

LineParticle::LineParticle(vector2f from, vector2f to, sf::Color color, int lifetime)
    : from(from), to(to), color(color), lifetime(lifetime), timeLeft(lifetime), dead(false)
{}
void LineParticle::iterate()
{
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

void drawSelectionBox(sf::RenderWindow *window, vector2i p1, vector2i p2)
{
    int rectLeft = min(p1.x, p2.x);
    int rectRight = max(p1.x, p2.x);
    int rectBottom = max(p1.y, p2.y);
    int rectTop = min(p1.y, p2.y);
    int width = rectRight - rectLeft;
    int height = rectBottom - rectTop;

    sf::RectangleShape rect(sf::Vector2f(width, height));
    rect.setPosition(rectLeft, rectTop);
    rect.setOutlineColor(sf::Color::Green);
    rect.setOutlineThickness(1);
    rect.setFillColor(sf::Color::Transparent);
    
    window->draw(rect);
}

void drawCursorOrSelectionBox(sf::RenderWindow *window, UI ui, int playerId)
{
    vector2i mousePos = vector2i(sf::Mouse::getPosition(*window).x, sf::Mouse::getPosition(*window).y);

    if (ui.maybeSelectionBoxStart)
    {
        drawSelectionBox(window, *ui.maybeSelectionBoxStart, mousePos);
    }
    else
    {
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
            case UI::SpawnBeacon:
                drawTargetCursor(window, mousePos, sf::Color::Yellow);
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
            case UI::Scuttle:
                if (ui.mouseoverEntity)
                {
                    if (getAllianceType(playerId, ui.mouseoverEntity) == Owned || ui.mouseoverEntity->typechar() == GOLDPILE_TYPECHAR)
                    {
                        drawBracketsCursor(window, mousePos, sf::Color::Yellow);
                    }
                    else
                    {
                        drawBracketsCursor(window, mousePos, sf::Color(100, 100, 100));
                    }
                }
                else
                {
                    drawBracketsCursor(window, mousePos, sf::Color(100, 100, 100));
                }
                break;
        }
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
            case Owned:
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

const int HOTKEY_BOX_WIDTH = 60;
const int HOTKEY_BOX_SPACING = 10;
const int HOTKEY_BOTTOMROW_INDENT = 18;

void drawHotkey(sf::RenderWindow *window, vector2i drawPos, InterfaceCmdWithState *interfaceCmdWithState, unsigned char keyChar, vector<string> cmdNameLines)
{
    sf::Color mainColor = interfaceCmdWithState->eligible ? sf::Color(100, 100, 255) : sf::Color(80, 80, 80);

    sf::RectangleShape rectShape(sf::Vector2f(HOTKEY_BOX_WIDTH, HOTKEY_BOX_WIDTH));
    rectShape.setPosition(drawPos.x, drawPos.y);
    rectShape.setFillColor(sf::Color::Transparent);
    rectShape.setOutlineColor(mainColor);
    rectShape.setOutlineThickness(1);
    window->draw(rectShape);

    sf::Text hotkeyText(string(1, keyChar), mainFont, 12);
    hotkeyText.setFillColor(mainColor);
    hotkeyText.setPosition(sf::Vector2f(drawPos.x + 2, drawPos.y-1));
    window->draw(hotkeyText);

    float lineHeight = 12;
    float allLinesHeight = lineHeight * cmdNameLines.size();
    float topLineYFromCenter = - allLinesHeight / 2;
    vector2f boxCenter(HOTKEY_BOX_WIDTH / 2, HOTKEY_BOX_WIDTH / 2);
    for (uint i=0; i<cmdNameLines.size(); i++)
    {
        sf::Text lineText(cmdNameLines[i], mainFont, 12);

        float width = lineText.getGlobalBounds().width;
        float lineXFromCenter = - width / 2;
        vector2f positionFromCenter(lineXFromCenter, topLineYFromCenter + (lineHeight * i));
        vector2f position = drawPos + boxCenter + positionFromCenter;
        lineText.setPosition(sf::Vector2f(position.x, position.y));

        lineText.setFillColor(mainColor);
        window->draw(lineText);
    }
}

void drawSpawnBeaconHotkey(sf::RenderWindow* window, UI *ui)
{
    vector2i drawPos(HOTKEY_BOX_SPACING, WINDOW_HEIGHT - (2*HOTKEY_BOX_SPACING + HOTKEY_BOX_WIDTH));
    drawHotkey(window, drawPos, &ui->spawnBeaconInterfaceCmdWithState, 'B', {"Spawn","Gateway"});
}

void drawUnitHotkeyHelp(sf::RenderWindow *window, UI *ui)
{
    const tuple<sf::Keyboard::Key, char, vector<string>> hotkeyInfo[] =
    {
        {sf::Keyboard::Q, 'Q', {"Build", "Prime"}},
        {sf::Keyboard::W, 'W', {"Build", "Fighter"}},
        {sf::Keyboard::E, 'E', {"Build", "Gateway"}},
        {sf::Keyboard::R, 'R', {}},
        {sf::Keyboard::A, 'A', {}},
        {sf::Keyboard::S, 'S', {}},
        {sf::Keyboard::D, 'D', {"Deposit"}},
        {sf::Keyboard::F, 'F', {"Scuttle"}}
    };

    int hotkeyHelpBoxWidth = HOTKEY_BOTTOMROW_INDENT + (4 * HOTKEY_BOX_WIDTH + 3 * HOTKEY_BOX_SPACING) + 20;
    int hotkeyHelpBoxHeight = (2 * HOTKEY_BOX_WIDTH + HOTKEY_BOX_SPACING) + 20;
    vector2i hotkeyHelpBoxUpperLeft = vector2f(10, WINDOW_HEIGHT - (hotkeyHelpBoxHeight + 10));

    sf::RectangleShape hotkeyHelpBoudingRect(sf::Vector2f(hotkeyHelpBoxWidth, hotkeyHelpBoxHeight));
    hotkeyHelpBoudingRect.setPosition(sf::Vector2f(hotkeyHelpBoxUpperLeft.x, hotkeyHelpBoxUpperLeft.y));
    hotkeyHelpBoudingRect.setFillColor(sf::Color(0, 0, 0, 200));
    hotkeyHelpBoudingRect.setOutlineColor(sf::Color(150, 150, 200));
    hotkeyHelpBoudingRect.setOutlineThickness(1);
    window->draw(hotkeyHelpBoudingRect);
    
    for (uint i=0; i<8; i++)
    {
        sf::Keyboard::Key keyCode = get<0>(hotkeyInfo[i]);
        char keyChar = get<1>(hotkeyInfo[i]);
        vector<string> cmdNameLines = get<2>(hotkeyInfo[i]);

        InterfaceCmdWithState* interfaceCmdWithState(NULL);
        for (uint j=0; j<ui->unitInterfaceCmdsWithState.size(); j++)
        {
            if (ui->unitInterfaceCmdsWithState[j].interfaceCmd->getKey() == keyCode)
            {
                interfaceCmdWithState = &ui->unitInterfaceCmdsWithState[j];
                break;
            }
        }
        if (!interfaceCmdWithState)
            continue;

        vector2i drawPosOffset;
        if (i < 4)
        {
            drawPosOffset = vector2i(i * (HOTKEY_BOX_WIDTH + HOTKEY_BOX_SPACING), 0);
        }
        else
        {
            drawPosOffset = vector2i(HOTKEY_BOTTOMROW_INDENT + (i-4) * (HOTKEY_BOX_WIDTH + HOTKEY_BOX_SPACING), (HOTKEY_BOX_WIDTH + HOTKEY_BOX_SPACING));
        }

        drawHotkey(window, hotkeyHelpBoxUpperLeft + vector2i(10, 10) + drawPosOffset, interfaceCmdWithState, keyChar, cmdNameLines);
    }
}

void drawEscapeQuitText(sf::RenderWindow* window, uint escapeTextCountdown, int countdownToQuitOrNeg1)
{
    sf::Text escapeQuitText("Hold Escape to quit.", mainFont, 40);
    sf::Text extraText("This will automatically withdraw your balance", mainFont, 20);

    int alpha = ((float)escapeTextCountdown / ESCAPE_TO_QUIT_TEXT_LIFE) * 255;
    escapeQuitText.setFillColor(sf::Color(255, 255, 255, alpha));
    extraText.setFillColor(sf::Color(255, 255, 255, alpha));

    escapeQuitText.setPosition(30, 30);
    extraText.setPosition(30, 90);

    window->draw(escapeQuitText);
    window->draw(extraText);

    if (countdownToQuitOrNeg1 >= 0)
    {
        float fractionLeft = (1 - ((float)countdownToQuitOrNeg1 / ESCAPE_TO_QUIT_TICKS));
        float progressBarMaxWidth = escapeQuitText.getGlobalBounds().width;
        float progressBarWidth = fractionLeft * progressBarMaxWidth;
        sf::RectangleShape progressBar(sf::Vector2f(progressBarWidth, 10));
        sf::RectangleShape maxProgressBar(sf::Vector2f(progressBarMaxWidth, 10));
        
        progressBar.setPosition(30, 80);
        maxProgressBar.setPosition(30, 80);

        int white = ((1-fractionLeft) * 255);
        sf::Color fillColor(255, white, white);

        progressBar.setFillColor(fillColor);
        maxProgressBar.setOutlineColor(sf::Color::White);
        maxProgressBar.setOutlineThickness(1);
        maxProgressBar.setFillColor(sf::Color::Transparent);
        window->draw(progressBar);
        window->draw(maxProgressBar);
    }
}

void display(sf::RenderWindow *window, Game *game, UI ui, ParticlesContainer *particles, int playerIdOrNegativeOne)
{
    drawBackground(window, ui.camera);

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
                    if (prime->goldTransferState == Pulling)
                    {
                        if (optional<vector2f> maybeTargetPos = prime->getTarget().getPointUnlessTargetDeleted(*game))
                        {
                            vector2f targetPos = *maybeTargetPos;
                            particles->addParticle(boost::shared_ptr<Particle>(new Particle(targetPos, Target(prime->ref), sf::Color::Yellow)));
                        }
                    }
                    else if (prime->goldTransferState == Pushing)
                    {
                        particles->addParticle(boost::shared_ptr<Particle>(new Particle(prime->pos, prime->getTarget(), sf::Color::Yellow)));
                    }
                }
                if (auto gateway = boost::dynamic_pointer_cast<Gateway, Entity>(game->entities[i]))
                {
                    if (auto targetEntity = entityRefToPtrOrNull(*game, gateway->maybeTargetEntity))
                    {
                        switch (gateway->inGameTransferState)
                        {
                            case NoGoldTransfer:
                            {
                                // no particles needed
                            }
                            break;
                            case Pushing:
                            {
                                particles->addParticle(boost::shared_ptr<Particle>(new Particle(gateway->pos, Target(targetEntity), sf::Color::Yellow)));
                            }
                            break;
                            case Pulling:
                            {
                                particles->addParticle(boost::shared_ptr<Particle>(new Particle(targetEntity->pos, Target(gateway), sf::Color::Yellow)));
                            }
                            break;
                        }
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
                            relativeShotStartPos = FIGHTER_SHOT_OFFSET;
                        }
                        else
                        {
                            vector2f reversedShotOffset(FIGHTER_SHOT_OFFSET);
                            reversedShotOffset.y *= -1;
                            relativeShotStartPos = reversedShotOffset;
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
    for (uint i=0; i<ui.selectedUnits.size(); i++)
    {
        drawSelectionCircleAroundEntity(window, ui.camera, ui.selectedUnits[i]);
    }

    drawUnitDroppableValues(window, game, ui, playerIdOrNegativeOne);

    vector<sf::String> outputStrings;

    if (playerIdOrNegativeOne >= 0)
    {
        uint playerId = playerIdOrNegativeOne;
        outputStrings.push_back("Account balance: " + game->players[playerId].credit.getDollarString());
    }

    drawOutputStrings(window, outputStrings);

    bool playerOwnsUnits(false);
    for (uint i=0; i<game->entities.size(); i++)
    {
        if (game->entities[i])
            if (getAllianceType(playerIdOrNegativeOne, game->entities[i]) == Owned)
            {
                playerOwnsUnits = true;
                break;
            }
    }
    if (playerOwnsUnits)
        drawUnitHotkeyHelp(window, &ui);
    else
        drawSpawnBeaconHotkey(window, &ui);

    if (ui.escapeTextCountdownOrNeg1 >= 0)
    {
        drawEscapeQuitText(window, (uint)ui.escapeTextCountdownOrNeg1, ui.countdownToQuitOrNeg1);
    }

    drawCursorOrSelectionBox(window, ui, playerIdOrNegativeOne);
    
    window->display();
}
#include <boost/shared_ptr.hpp>
#include <iostream>
#include <fstream>
#include <SFML/Graphics.hpp>
#include "engine.h"
#include "graphics.h"
#include "common.h"
#include "config.h"
#include "tutorial.h"
#include "graphics_helpers.h"
#include "ui_elements.h"
#include "particles.h"

using namespace std;

const sf::Color GATEWAY_MAIN_COLOR = sf::Color(100,100,255);
const sf::Color GATEWAY_INNEROUTLINE_COLOR = sf::Color(0,0,255);

const sf::Color FUNDS_LINE_COLOR = sf::Color(255, 255, 0, 100);
const sf::Color BUILD_JOB_LINE_COLOR = sf::Color(100, 100, 255, 100);
const sf::Color SCUTTLE_JOB_LINE_COLOR = sf::Color(255, 100, 100, 100);

const sf::Color FIGHTER_BARREL_COLOR = sf::Color::Red;

const sf::Color TURRET_MAIN_COLOR = sf::Color(255, 100, 100);

const float ENERGY_LINE_SEGMENT_LENGTH = 10;
const float ENERGY_LINE_PERTURB_AMOUNT = 3;

float radToDeg(float rad)
{
    return rad * (180 / M_PI);
}

void loadFonts(sf::Font* mainFont, sf::Font* tutorialFont)
{
    if (!mainFont->loadFromFile("Andale_Mono.ttf"))
        throw runtime_error("Can't load main font");
    if (!tutorialFont->loadFromFile("NotoSansCJK-Regular.ttc"))
        throw runtime_error("Can't load tutorial font");
}

sf::RenderWindow* setupGraphics(bool fullscreen, bool smallScreen)
{
    // choose a good videomode
    sf::VideoMode chosenMode;
    if (smallScreen)
    {
        chosenMode = sf::VideoMode(800, 600, 24);
    }
    else if (!fullscreen)
    {
        chosenMode = sf::VideoMode(1280, 720, 24);
    }
    else
    {
        bool modeFound(false);

        auto modes = sf::VideoMode::getFullscreenModes();
        for (unsigned int i = 0; i < modes.size(); i++)
        {
            if (modes[i].width == 1920 && modes[i].height == 1080)
            {
                modeFound = true;
                chosenMode = modes[i];
                break;
            }
        }
        if (!modeFound) // gotta lower our standards!
        {
            for (unsigned int i = 0; i < modes.size(); i++)
            {
                if (modes[i].width <= 1920)
                {
                    modeFound = true;
                    chosenMode = modes[i];
                    break;
                }
            }
        }
        if (!modeFound)
        {
            // weird, but okay. Just choose the first mode found.
            modeFound = true;
            chosenMode = modes[0];
        }
        chosenMode.bitsPerPixel = 24;
    }
    updateScreenDimensions(vector2i(chosenMode.width, chosenMode.height));

    auto flags =
        fullscreen ? sf::Style::Close | sf::Style::Fullscreen
                   : sf::Style::Close | sf::Style::Titlebar;

    sf::RenderWindow* window = new sf::RenderWindow(chosenMode, "Coinfight Client", flags);
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
    vector2fl centerOfMapScreenPos = gamePosToScreenPos(camera, vector2fp::zero);

    sf::CircleShape circle(1, 40);
    circle.setFillColor(sf::Color::Transparent);
    circle.setOutlineColor(backgroundCirclesColor);
    circle.setOutlineThickness(1);
    circle.setPosition(centerOfMapScreenPos.x, centerOfMapScreenPos.y);

    for (unsigned int i=0; i<10; i++)
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
            unsigned int numSpokes = pow(2, i);

            sf::VertexArray lines(sf::Lines, numSpokes*2);
            for (unsigned int i=0; i<numSpokes; i++)
            {
                float angle = ((float)i / numSpokes) * 2 * M_PI;
                vector2fl from = gamePosToScreenPos(camera, vector2fp(composeVector2fl(angle, radius)));
                vector2fl to = gamePosToScreenPos(camera, vector2fp(composeVector2fl(angle, nextRadius)));

                lines[i*2].position = toSFVec(from);
                lines[i*2+1].position = toSFVec(to);
                lines[i*2].color = backgroundCirclesColor;
                lines[i*2+1].color = backgroundSpokeEndColor;
            }

            window->draw(lines);
        }
    }
}

void drawEnergyLine(sf::RenderWindow *window, CameraState camera, vector2fp fromGamePos, vector2fp toGamePos, sf::Color color)
{
    vector2fl from = gamePosToScreenPos(camera, fromGamePos);
    vector2fl to = gamePosToScreenPos(camera, toGamePos);

    vector2fl lineVec = (to - from);
    float length = lineVec.getMagnitude();

    // travel along length and draw segments with perturbed points

    int numNonEndPoints = length / ENERGY_LINE_SEGMENT_LENGTH; // i.e. if length is 1 and ENERGY(..) is 1.2, this would be zero because the only points would be the end points.

    sf::VertexArray lines(sf::LineStrip, numNonEndPoints + 2);

    lines[0].position = toSFVec(from);
    lines[0].color = color;
    for (int i=0; i<numNonEndPoints; i++)
    {
        int vertexI = i + 1;
        float relativeLinePosition = ((float)vertexI / (numNonEndPoints + 1));
        vector2fl pointOnLine = from + (lineVec * relativeLinePosition);
        vector2fl perturbed = pointOnLine + randomVectorWithMagnitude(ENERGY_LINE_PERTURB_AMOUNT);

        lines[vertexI].position = toSFVec(perturbed);
        lines[vertexI].color = color;
    }
    lines[numNonEndPoints + 1].position = toSFVec(to);
    lines[numNonEndPoints + 1].color = color;

    window->draw(lines);
}

void drawGoldPile(sf::RenderWindow *window, boost::shared_ptr<GoldPile> goldPile, vector2fl drawPos)
{
    float width = ceil(sqrt(goldPile->gold.getInt() / 150.0)) + 5;
    float height = width * .4;
    if (width > 1)
    {
        sf::Color goldBottomColor(sf::Color(255, 130, 0));
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

const sf::Color UNIT_OUTLINE_COLOR(100, 100, 100);

void drawBeacon(sf::RenderWindow *window, vector2fl drawPos, sf::Color teamColor, unsigned int alpha)
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

void drawGateway(sf::RenderWindow *window, vector2fl drawPos, sf::Color teamColor, unsigned int alpha)
{
    sf::Color fillColorFaded(GATEWAY_MAIN_COLOR.r, GATEWAY_MAIN_COLOR.g, GATEWAY_MAIN_COLOR.b, alpha);

    sf::CircleShape outerHex(15, 6);
    outerHex.setOrigin(15, 15);
    outerHex.setFillColor(fillColorFaded);
    outerHex.setOutlineColor(UNIT_OUTLINE_COLOR);
    outerHex.setOutlineThickness(1);
    outerHex.setPosition(drawPos.x, drawPos.y);
    outerHex.setRotation(90);

    window->draw(outerHex);

    drawBeacon(window, drawPos, teamColor, alpha);
}

void drawPrime(sf::RenderWindow *window, boost::shared_ptr<Prime> prime, vector2fl drawPos, float drawRotation, unsigned int alpha)
{
    sf::Transform transform = sf::Transform();
    transform.translate(drawPos.x, drawPos.y);
    transform.rotate(radToDeg(drawRotation));

    sf::Color teamColor = prime->getTeamColor();
    sf::Color mainPrimeColor(teamColor.r, teamColor.g, teamColor.b, alpha);

    float borderThickness = 2;
    float primeCavityRadius = static_cast<float>(PRIME_RADIUS) - borderThickness;

    sf::VertexArray wingPoints(sf::Triangles, 3);
    wingPoints[0].position = sf::Vector2f(0, static_cast<float>(PRIME_RADIUS));
    wingPoints[1].position = sf::Vector2f(-static_cast<float>(PRIME_RADIUS)*1.4, static_cast<float>(PRIME_RADIUS));
    wingPoints[2].position = toSFVec(composeVector2fl(0.8 * M_PI, static_cast<float>(PRIME_RADIUS)));
    wingPoints[0].color = wingPoints[1].color = wingPoints[2].color = mainPrimeColor;
    window->draw(wingPoints, transform);

    wingPoints[0].position.y *= -1;
    wingPoints[1].position.y *= -1;
    wingPoints[2].position.y *= -1;
    window->draw(wingPoints, transform);

    sf::CircleShape structureOutline(primeCavityRadius);
    structureOutline.setOrigin(sf::Vector2f(primeCavityRadius, primeCavityRadius));
    structureOutline.setFillColor(sf::Color::Transparent);
    structureOutline.setOutlineColor(UNIT_OUTLINE_COLOR);
    structureOutline.setOutlineThickness(1);
    window->draw(structureOutline, transform);

    sf::CircleShape thickBorder(structureOutline);
    thickBorder.setOutlineColor(mainPrimeColor);
    thickBorder.setOutlineThickness(borderThickness);

    window->draw(thickBorder, transform);

    float heldGoldRatio = static_cast<float>(prime->getHeldGoldRatio());
    if (heldGoldRatio > 0)
    {
        float innerGoldRadius = 0.5 + (sqrt(heldGoldRatio) * (primeCavityRadius - 0.5));
        // radius is at least 0.5 (so any gold draws a circle of width 1) and scales up to primeCavityRadius

        sf::CircleShape heldGoldCircle(innerGoldRadius);
        heldGoldCircle.setOrigin(sf::Vector2f(innerGoldRadius, innerGoldRadius));
        heldGoldCircle.setPosition(toSFVec(drawPos));
        heldGoldCircle.setFillColor(sf::Color::Yellow);

        window->draw(heldGoldCircle);
    }
}

void drawFighter(sf::RenderWindow *window, vector2fl drawPos, float rotation, sf::Color teamColor, unsigned int alpha)
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
    gunBarrelPoints[2].position = toSFVec(COMBATUNIT_SHOT_OFFSET);
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
    lines[0].color = UNIT_OUTLINE_COLOR;
    lines[1].color = UNIT_OUTLINE_COLOR;
    lines[2].color = UNIT_OUTLINE_COLOR;
    lines[3].color = UNIT_OUTLINE_COLOR;
    lines[4].color = UNIT_OUTLINE_COLOR;

    transform = sf::Transform();
    transform.translate(drawPos.x, drawPos.y);
    transform.rotate(radToDeg(rotation));
    window->draw(lines, transform);
}

void drawTurret(sf::RenderWindow *window, vector2fl drawPos, float rotation, sf::Color teamColor, unsigned int alpha)
{
    sf::Color fillColorFaded(TURRET_MAIN_COLOR.r,TURRET_MAIN_COLOR.g, TURRET_MAIN_COLOR.b, alpha);

    int halfWidth = int(TURRET_RADIUS * fixed32(0.85));
    int width = halfWidth * 2;

    sf::RectangleShape box(sf::Vector2f(width, width));
    box.setOrigin(halfWidth, halfWidth);
    box.setFillColor(fillColorFaded);
    box.setOutlineColor(UNIT_OUTLINE_COLOR);
    box.setOutlineThickness(1);
    box.setPosition(drawPos.x, drawPos.y);

    window->draw(box);

    drawFighter(window, drawPos, rotation, teamColor, alpha);
}

void drawUnit(sf::RenderWindow *window, boost::shared_ptr<Unit> unit, vector2fl drawPos)
{
    sf::Color teamColor = unit->getTeamColor(); // may be modified later if unit is not yet active
    float drawRotation = -unit->angle_view;

    unsigned int fadedAlpha;
    if (!unit->isActive())
    {
        fadedAlpha = static_cast<float>(unit->getBuiltRatio()) * 0.8 * 255;
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
        drawPrime(window, prime, drawPos, drawRotation, fadedAlpha);
    }
    else if (auto fighter = boost::dynamic_pointer_cast<Fighter, Unit>(unit))
    {
        drawFighter(window, drawPos, drawRotation, teamColor, fadedAlpha);
    }
    else if (auto turret = boost::dynamic_pointer_cast<Turret, Unit>(unit))
    {
        drawTurret(window, drawPos, drawRotation, teamColor, fadedAlpha);
    }
    else {
        throw runtime_error("No drawing code implemented for unit " + unit->getTypename() + ".");
    }
}

void drawEntity(sf::RenderWindow *window, boost::shared_ptr<Entity> entity, vector2i drawPos)
{
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
        throw runtime_error("No drawing code implemented for entity " + entity->getTypename() + ".");
    }
}

vector2i scaledDownGamePosWithZCorrection(vector2fl gamePos, float scaleDownFactor)
{
    return vector2i(
        (gamePos.x > 0 ?
            (int)(gamePos.x / scaleDownFactor)
            : (int)((gamePos.x / scaleDownFactor) - 1)
        ), // otherwise everything between -1 and 1 shows up at col (and below, row) 0
        (gamePos.y > 0 ?
            (int)(gamePos.y / scaleDownFactor)
            : (int)((gamePos.y / scaleDownFactor) - 1)
        )
    );
}

void drawEntitySymbolOnMinimap(sf::RenderWindow *window, boost::shared_ptr<Entity> entity, optional<uint8_t> maybeViewingPlayerId, float zoomOutFactor)
{
    vector2i minimapPos = scaledDownGamePosWithZCorrection(vector2fl(entity->getPos()), zoomOutFactor);
    minimapPos.y *= -1;

    sf::RectangleShape pixel(sf::Vector2f(1,1));

    pixel.setOrigin(sf::Vector2f(0.5, 0.5));
    pixel.setPosition(sf::Vector2f(toSFVec(minimapPos + screenCenter)));
    pixel.setFillColor(entity->getTeamOrPrimaryColor());

    window->draw(pixel);
}

void drawAccountBalance(sf::RenderWindow *window, Coins *playerBalance, sf::Font font, sf::Color balanceTextColor, sf::Vector2f upperLeft, bool drawWalletHints)
{
    if (drawWalletHints)
    {
        vector<sf::Text> hints
            {sf::Text(sf::String("This can be spent or captured via Gateways"), font, 16),
            sf::Text(sf::String("and withdraws/deposits into xDai."), font, 16)
            };

        sf::Transform hintsTransform;
        hintsTransform.translate(upperLeft);
        hintsTransform.translate(sf::Vector2f(120, 20));
        for (unsigned int i=0; i<hints.size(); i++)
        {
            hints[i].setFillColor(sf::Color(150, 150, 150));
            window->draw(hints[i], hintsTransform);
            hintsTransform.translate(sf::Vector2f(0, hints[i].getLocalBounds().height + 7));
        }
    }

    int textSpacing = 10;

    sf::Text title(sf::String("Wallet"), font, 24);

    sf::Text balance(sf::String(playerBalance->getDollarString()), font, 30);
    balance.setFillColor(balanceTextColor);

    sf::Transform transform;
    transform.translate(upperLeft);

    window->draw(title, transform);

    transform.translate(sf::Vector2f(0, title.getLocalBounds().height + textSpacing));
    window->draw(balance, transform);
}

void drawOutputStrings(sf::RenderWindow *window, vector<sf::String> strings, sf::Font font)
{
    for (unsigned int i=0; i<strings.size(); i++)
    {
        sf::Text text(strings[i], font, 16);
        text.setFillColor(sf::Color(150, 150, 150));

        float width = text.getLocalBounds().width;
        int x, y;
        x = window->getSize().x - width - 10;
        y = 6 + i * 20;
        text.setPosition(sf::Vector2f(x, y));

        window->draw(text);
    }
}

void drawCircleAround(sf::RenderWindow *window, vector2i screenPos, unsigned int radius, unsigned int thickness, sf::Color color)
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

void drawGhostBuilding(sf::RenderWindow *window, const GameUI* ui, vector2i mousePos)
{
    window->setMouseCursorVisible(false);

    drawEntity(window, ui->ghostBuilding, mousePos);
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

void drawCursorOrSelectionBox(sf::RenderWindow *window, GameUI* ui, optional<uint8_t> maybePlayerId)
{
    vector2i mousePos = vector2i(sf::Mouse::getPosition(*window).x, sf::Mouse::getPosition(*window).y);

    if (ui->maybeSelectionBoxStart)
    {
        drawSelectionBox(window, *ui->maybeSelectionBoxStart, mousePos);
    }
    else
    {
        // depending on cmdState and mouseoverEntity
        switch (ui->cmdState)
        {
            case GameUI::Default:
                if (ui->mouseoverEntity)
                {
                    if (getAllianceType(maybePlayerId, ui->mouseoverEntity) == Foreign)
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
            case GameUI::SpawnBeacon:
                drawTargetCursor(window, mousePos, sf::Color::Yellow);
                break;
            case GameUI::AttackAbsorb:
                if (ui->mouseoverEntity)
                {
                    drawBracketsCursor(window, mousePos, sf::Color::Red);
                }
                else
                {
                    drawTargetCursor(window, mousePos, sf::Color::Red);
                }
                break;
            case GameUI::Deposit:
                if (ui->mouseoverEntity)
                {
                    drawBracketsCursor(window, mousePos, sf::Color::Blue);
                }
                else
                {
                    drawTargetCursor(window, mousePos, sf::Color::Blue);
                }
                break;
            case GameUI::Fetch:
                if (ui->mouseoverEntity)
                {
                    if (getAllianceType(maybePlayerId, ui->mouseoverEntity) == Owned || ui->mouseoverEntity->typechar() == GOLDPILE_TYPECHAR)
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
                    if (filterForTypeKeepContainer<Prime, Unit>(ui->selectedUnits).size() > 0)
                    {
                        drawTargetCursor(window, mousePos, sf::Color::Yellow);
                    }
                    else
                    {
                        drawBracketsCursor(window, mousePos, sf::Color(100, 100, 100));
                    }
                }
                break;
            case GameUI::Build:
                drawGhostBuilding(window, ui, mousePos);
                break;
        }
    }
}

void drawSelectionCircleAroundEntity(sf::RenderWindow *window, CameraState camera, boost::shared_ptr<Entity> entity)
{
    drawCircleAround(window, gamePosToScreenPos(camera, entity->getPos()), 15, 1, sf::Color::Green);
}

void drawUnitDroppableValues(sf::RenderWindow *window, Game *game, GameUI* ui, optional<uint8_t> maybePlayerId, sf::Font font)
{
    for (unsigned int i=0; i<game->entities.size(); i++)
    {
        if (!game->entities[i])
            continue;
        Coins *displayAboveCoins = NULL;
        Coins *displayBelowCoins = NULL;

        sf::Color topTextColor;
        switch (getAllianceType(maybePlayerId, game->entities[i]))
        {
            case Owned:
                topTextColor = sf::Color::Green;
                break;
            case Foreign:
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
            // displayAboveCoins = &unit->goldInvested;
            // if (auto prime = boost::dynamic_pointer_cast<Prime, Unit>(unit))
            // {
            //     displayBelowCoins = &prime->heldGold;
            // }
        }

        vector2fp entityPos = game->entities[i]->getPos();
        if (displayAboveCoins)
        {
            sf::Text aboveText(displayAboveCoins->getDollarString(), font, 16);
            sf::FloatRect textRec = aboveText.getLocalBounds();

            vector2fp textGamePos = entityPos + vector2fp(fixed32(0), fixed32(30));
            vector2fl textScreenPos = gamePosToScreenPos(ui->camera, textGamePos);

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
            sf::Text belowText(displayBelowCoins->getDollarString(), font, 16);
            sf::FloatRect textRec = belowText.getLocalBounds();

            vector2fp textGamePos = entityPos + vector2fp(fixed32(0), fixed32(-20));
            vector2fl textScreenPos = gamePosToScreenPos(ui->camera, textGamePos);

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

void drawUnitHealthBars(sf::RenderWindow* window, Game* game, GameUI* ui, optional<uint8_t> maybePlayerId)
{
    for (unsigned int i=0; i<game->entities.size(); i++)
    {
        if (!game->entities[i])
            continue;
        
        if (auto unit = boost::dynamic_pointer_cast<Unit, Entity>(game->entities[i]))
        {
            sf::Color barColorFill, barColorOutline;
            switch (getAllianceType(maybePlayerId, unit))
            {
                case Owned:
                    barColorFill = sf::Color(0, 255, 0);
                    barColorOutline = sf::Color(0, 100, 0);
                    break;
                case Foreign:
                    barColorFill = sf::Color(255, 0, 0);
                    barColorOutline = sf::Color(100, 0, 0);
                    break;
                case Neutral:
                    barColorFill = sf::Color(0, 0, 255);
                    barColorOutline = sf::Color(0, 0, 100);
                    break;
            }

            vector2fp healthBarPos = unit->getPos() - vector2fp(fixed32(0), unit->getRadius() + 8);

            float healthBarLength = static_cast<float>(unit->getRadius()) * 2;

            sf::RectangleShape healthBar(sf::Vector2f(healthBarLength, 6)); // will be used to draw both outline and fill
            healthBar.setOrigin(healthBar.getLocalBounds().width / 2, healthBar.getLocalBounds().height / 2);
            healthBar.setPosition(sf::Vector2f(toSFVec(gamePosToScreenPos(ui->camera, healthBarPos))));

            healthBar.setOutlineColor(barColorOutline);
            healthBar.setFillColor(sf::Color(100, 100, 100));
            healthBar.setOutlineThickness(1);

            window->draw(healthBar);

            float healthRatio = (float)(unit->getEffectiveHealth()) / (float)(unit->getMaxHealth());
            healthBar.setFillColor(barColorFill);
            healthBar.setOutlineThickness(0);
            healthBar.setSize(sf::Vector2f(healthRatio * healthBarLength, 6));

            window->draw(healthBar);
        }
    }
}

void drawArrow(sf::RenderWindow* window, GameUI* ui, vector2fp drawPos, bool pointingUp, sf::Color color)
{
    sf::ConvexShape arrowPoint;
    arrowPoint.setPointCount(3);
    arrowPoint.setPoint(0, sf::Vector2f(0, -4));
    arrowPoint.setPoint(1, sf::Vector2f(-5, 4));
    arrowPoint.setPoint(2, sf::Vector2f(5, 4));

    arrowPoint.setFillColor(color);
    arrowPoint.setPosition(sf::Vector2f(toSFVec(gamePosToScreenPos(ui->camera, drawPos))));

    if (!pointingUp)
    {
        arrowPoint.setRotation(180);
    }

    window->draw(arrowPoint);
}

const int HOTKEY_BOX_WIDTH = 60;
const int HOTKEY_BOX_SPACING = 10;
const int HOTKEY_BOTTOMROW_INDENT = 18;

void drawHotkey(sf::RenderWindow *window, vector2i drawPos, const InterfaceCmd &interfaceCmd, sf::Font font)
{
    auto hotkeyInfo = interfaceCmd.getHotkeyInfo();
    char keyChar = get<1>(hotkeyInfo);
    vector<string> cmdNameLines = get<2>(hotkeyInfo);
    optional<coinsInt> maybeCost = get<3>(hotkeyInfo);

    sf::Color mainOutlineColor, nameColor, hotkeyTextColor, hotkeyBackgroundColor, hotkeyOutlineColor, costStringColor;
    if (interfaceCmd.eligible)
    {
        mainOutlineColor = sf::Color(100, 100, 255);
        nameColor = sf::Color::White;
        hotkeyTextColor = sf::Color::White;
        hotkeyBackgroundColor = sf::Color(100, 100, 255, 100);
        hotkeyOutlineColor = sf::Color(100, 100, 255);
        costStringColor = sf::Color::Yellow;
    }
    else
    {
        mainOutlineColor = nameColor = costStringColor = hotkeyOutlineColor = sf::Color(80, 80, 80);
        hotkeyBackgroundColor = sf::Color::Black;
        hotkeyTextColor = sf::Color(80, 80, 80);
    }
    sf::Color backgroundColor =
        interfaceCmd.visualFlashClock.getElapsedTime() < sf::seconds(0.06) || interfaceCmd.active ?
        sf::Color(150, 0, 0, 100) :
        sf::Color::Transparent ;

    sf::RectangleShape rectShape(sf::Vector2f(HOTKEY_BOX_WIDTH, HOTKEY_BOX_WIDTH));
    rectShape.setPosition(drawPos.x, drawPos.y);
    rectShape.setFillColor(backgroundColor);
    rectShape.setOutlineColor(mainOutlineColor);
    rectShape.setOutlineThickness(1);
    window->draw(rectShape);

    sf::Text hotkeyText(string(1, keyChar), font, 14);
    hotkeyText.setFillColor(hotkeyTextColor);

    // determine horizontal placement of hotkey
    int width = hotkeyText.getLocalBounds().width;
    int hSpaceLeft = 16 - width;
    int xOffset = hSpaceLeft / 2 - 1;
    // manual adjustment. Idk why this is needed but I just wanna fix it.
    if (keyChar == 'W' || keyChar == 'A')
    {
        xOffset += 1;
    }
    if (keyChar == 'E')
    {
        xOffset -= 1;
    }

    hotkeyText.setPosition(sf::Vector2f(drawPos.x + xOffset , drawPos.y-1));

    sf::RectangleShape hotkeyBackground(sf::Vector2f(16, 18));
    hotkeyBackground.setPosition(drawPos.x, drawPos.y);
    hotkeyBackground.setFillColor(hotkeyBackgroundColor);
    hotkeyBackground.setOutlineColor(hotkeyOutlineColor);
    hotkeyBackground.setOutlineThickness(1);

    window->draw(hotkeyBackground);
    window->draw(hotkeyText);

    if (maybeCost)
    {
        string coinsString = coinsIntToDollarString(*maybeCost);
        sf::Text costText(coinsString, font, 12);
        costText.setFillColor(costStringColor);

        int width = costText.getLocalBounds().width;
        costText.setPosition(sf::Vector2f((drawPos.x + HOTKEY_BOX_WIDTH) - (4 + width), drawPos.y-1));
        window->draw(costText);
    }

    float lineHeight = 12;
    float allLinesHeight = lineHeight * cmdNameLines.size();
    float topLineYFromCenter = - allLinesHeight / 2;
    vector2fl boxCenter(HOTKEY_BOX_WIDTH / 2, HOTKEY_BOX_WIDTH / 2);
    for (unsigned int i=0; i<cmdNameLines.size(); i++)
    {
        sf::Text lineText(cmdNameLines[i], font, 12);

        float width = lineText.getGlobalBounds().width;
        float lineXFromCenter = - width / 2;
        vector2fl positionFromCenter(lineXFromCenter, topLineYFromCenter + (lineHeight * i));
        vector2fl position = drawPos + boxCenter + positionFromCenter;
        lineText.setPosition(sf::Vector2f(position.x, position.y));

        lineText.setFillColor(nameColor);
        window->draw(lineText);
    }
}

void drawSpawnBeaconHotkey(sf::RenderWindow* window, GameUI *ui, sf::Font font)
{
    vector2i drawPos(HOTKEY_BOX_SPACING, screenDimensions.y - (2*HOTKEY_BOX_SPACING + HOTKEY_BOX_WIDTH));
    drawHotkey(window, drawPos, ui->spawnBeaconInterfaceCmd, font);
}

void drawUnitHotkeyHelp(sf::RenderWindow *window, GameUI *ui, sf::Font font)
{
    int hotkeyHelpBoxWidth = HOTKEY_BOTTOMROW_INDENT + (4 * HOTKEY_BOX_WIDTH + 3 * HOTKEY_BOX_SPACING) + 20;
    int hotkeyHelpBoxHeight = (2 * HOTKEY_BOX_WIDTH + HOTKEY_BOX_SPACING) + 20;
    vector2i hotkeyHelpBoxUpperLeft = vector2fl(10, screenDimensions.y - (hotkeyHelpBoxHeight + 10));

    sf::RectangleShape hotkeyHelpBoudingRect(sf::Vector2f(hotkeyHelpBoxWidth, hotkeyHelpBoxHeight));
    hotkeyHelpBoudingRect.setPosition(sf::Vector2f(hotkeyHelpBoxUpperLeft.x, hotkeyHelpBoxUpperLeft.y));
    hotkeyHelpBoudingRect.setFillColor(sf::Color(0, 0, 0, 200));
    hotkeyHelpBoudingRect.setOutlineColor(sf::Color(150, 150, 200));
    hotkeyHelpBoudingRect.setOutlineThickness(1);
    window->draw(hotkeyHelpBoudingRect);

    for (unsigned int i=0; i<ui->unitInterfaceCmds.size(); i++)
    {
        vector2i drawPosOffset;
        if (i < 4)
        {
            drawPosOffset = vector2i(i * (HOTKEY_BOX_WIDTH + HOTKEY_BOX_SPACING), 0);
        }
        else
        {
            drawPosOffset = vector2i(HOTKEY_BOTTOMROW_INDENT + (i-4) * (HOTKEY_BOX_WIDTH + HOTKEY_BOX_SPACING), (HOTKEY_BOX_WIDTH + HOTKEY_BOX_SPACING));
        }

        drawHotkey(window, hotkeyHelpBoxUpperLeft + vector2i(10, 10) + drawPosOffset, *ui->unitInterfaceCmds[i], font);
    }
}

void drawEscapeQuitText(sf::RenderWindow* window, unsigned int escapeTextCountdown, int countdownToQuitOrNeg1, sf::Font font)
{
    sf::Text escapeQuitText("Hold Escape to quit.", font, 40);

    int alpha = ((float)escapeTextCountdown / ESCAPE_TO_QUIT_TEXT_LIFE) * 255;
    escapeQuitText.setFillColor(sf::Color(255, 255, 255, alpha));

    escapeQuitText.setPosition(30, 100);

    window->draw(escapeQuitText);

    if (countdownToQuitOrNeg1 >= 0)
    {
        float fractionLeft = (1 - ((float)countdownToQuitOrNeg1 / ESCAPE_TO_QUIT_TICKS));
        float progressBarMaxWidth = escapeQuitText.getGlobalBounds().width;
        float progressBarWidth = fractionLeft * progressBarMaxWidth;
        sf::RectangleShape progressBar(sf::Vector2f(progressBarWidth, 10));
        sf::RectangleShape maxProgressBar(sf::Vector2f(progressBarMaxWidth, 10));

        progressBar.setPosition(30, 150);
        maxProgressBar.setPosition(30, 150);

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

void displayMinimap(sf::RenderWindow *window, Game *game, optional<uint8_t> maybePlayerId, vector2i minimapDimensions)
{
    sf::Color backgroundColor(0, 0, 50);
    window->clear(backgroundColor);

    float zoomOutFactor = 10;

    for (unsigned int i = 0; i < game->entities.size(); i++)
    {
        if (game->entities[i])
        {
            drawEntitySymbolOnMinimap(window, game->entities[i], maybePlayerId, zoomOutFactor);
        }
    }
}

void displayTutorial(sf::RenderWindow *window, Tutorial* tutorial, Game* game, GameUI* ui, int boxWidth, sf::Font font)
{
    sf::Transform transform;
    transform.translate(
        sf::Vector2f(
            screenDimensions.x - boxWidth - 10,
            10
        )
    );

    vector<string> preBarTextBlocks = get<0>(tutorial->currentStep()->getText(game, ui));
    vector<string> postBarTextBlocks = get<1>(tutorial->currentStep()->getText(game, ui));

    for (unsigned int i=0; i<preBarTextBlocks.size(); i++)
    {
        GH::wrapAndRenderTextWithTransform(window, preBarTextBlocks[i], &font, 13, sf::Color::White, boxWidth, &transform);
    }

    if (auto progress = tutorial->currentStep()->getProgress(game, ui))
    {
        if (*progress > 0)
        {
            float clampedProgress = max(0.f, min(1.f, *progress)); // clamp between 0 and 1

            float progressBarMaxWidth = boxWidth;
            float progressBarWidth = clampedProgress * progressBarMaxWidth;
            sf::RectangleShape progressBar(sf::Vector2f(progressBarWidth, 10));
            sf::RectangleShape maxProgressBar(sf::Vector2f(progressBarMaxWidth, 10));

            progressBar.setFillColor(sf::Color(150, 150, 255));
            maxProgressBar.setOutlineColor(sf::Color::White);
            maxProgressBar.setOutlineThickness(1);
            maxProgressBar.setFillColor(sf::Color::Transparent);

            window->draw(progressBar, transform);
            window->draw(maxProgressBar, transform);

            transform.translate(sf::Vector2f(0, 18));

            for (unsigned int i=0; i<postBarTextBlocks.size(); i++)
            {
                GH::wrapAndRenderTextWithTransform(window, postBarTextBlocks[i], &font, 13, sf::Color::White, boxWidth, &transform);
            }
        }
    }
    
    if (tutorial->currentStep()->isReadyToFinish(game, ui) && tutorial->currentStep()->waitForEnter)
    {
        GH::wrapAndRenderTextWithTransform(window, "Press enter to continue", &font, 16, sf::Color::White, boxWidth, &transform);
    }
}

coinsInt lastPlayerCredit = 0;
void display(sf::RenderWindow *window, Game *game, GameUI* ui, optional<uint8_t> maybePlayerId, Tutorial* tutorial, sf::Font mainFont, sf::Font tutorialFont, bool drawWalletHints)
{
    if (ui->minimapEnabled)
    {
        displayMinimap(window, game, maybePlayerId, screenDimensions);
    }
    else {
        drawBackground(window, ui->camera);

        ui->particles.drawParticles(window, ui->camera);
        ui->particles.iterateParticles(*game);

        for (unsigned int i = 0; i < game->entities.size(); i++)
        {
            if (game->entities[i])
            {
                vector2i drawPos = gamePosToScreenPos(ui->camera, vector2i(game->entities[i]->getPos()));
                drawEntity(window, game->entities[i], drawPos);

                // add some effects for gold transfer and building/scuttling
                if (auto prime = boost::dynamic_pointer_cast<Prime, Entity>(game->entities[i]))
                {
                    if (prime->isActive())
                    {
                        if (auto flowFromEntity = get<0>(prime->goldFlowFrom_view))
                        {
                            if (game->frame % 3 == 0)
                            {
                                ui->particles.addParticle(boost::shared_ptr<Particle>(new Particle(vector2fl(flowFromEntity->getPos()), Target(prime->getRefOrThrow()), sf::Color::Yellow)));
                            }

                            bool scuttling = get<1>(prime->goldFlowFrom_view);
                            if (scuttling)
                            {
                                drawEnergyLine(window, ui->camera, flowFromEntity->getPos(), prime->getPos(), sf::Color::Red);
                            }
                        }
                        if (auto flowToEntity = get<0>(prime->goldFlowTo_view)) 
                        {
                            if (game->frame % 3 == 0)
                            {
                                ui->particles.addParticle(boost::shared_ptr<Particle>(new Particle(vector2fl(prime->getPos()), Target(flowToEntity->getRefOrThrow()), sf::Color::Yellow)));
                            }

                            bool building = get<1>(prime->goldFlowTo_view);
                            if (building)
                            {
                                drawEnergyLine(window, ui->camera, flowToEntity->getPos(), prime->getPos(), sf::Color::Blue);
                            }
                        }
                    }
                }
                if (auto gateway = boost::dynamic_pointer_cast<Gateway, Entity>(game->entities[i]))
                {
                    if (gateway->isActive())
                    {
                        // draw active energy lines
                        if (auto flowFromEntity = get<0>(gateway->goldFlowFrom_view))
                        {
                            if (game->frame % 3 == 0)
                            {
                                ui->particles.addParticle(boost::shared_ptr<Particle>(new Particle(vector2fl(flowFromEntity->getPos()), Target(gateway->getRefOrThrow()), sf::Color::Yellow)));
                            }

                            bool scuttling = get<1>(gateway->goldFlowFrom_view);
                            if (scuttling)
                            {
                                drawEnergyLine(window, ui->camera, flowFromEntity->getPos(), gateway->getPos(), sf::Color::Red);
                            }
                        }
                        if (auto flowToEntity = get<0>(gateway->goldFlowTo_view)) 
                        {
                            if (game->frame % 3 == 0)
                            {
                                ui->particles.addParticle(boost::shared_ptr<Particle>(new Particle(vector2fl(gateway->getPos()), Target(flowToEntity->getRefOrThrow()), sf::Color::Yellow)));
                            }

                            bool building = get<1>(gateway->goldFlowTo_view);
                            if (building)
                            {
                                drawEnergyLine(window, ui->camera, flowToEntity->getPos(), gateway->getPos(), sf::Color::Blue);
                            }
                        }
                    }
                }

                // combatUnit shots
                if (auto combatUnit = boost::dynamic_pointer_cast<CombatUnit, Entity>(game->entities[i]))
                {
                    if (combatUnit->animateShot_view != CombatUnit::None)
                    {
                        vector2fl targetPos(combatUnit->lastShotTargetPos_view);
                        vector2fl relativeShotStartPos;
                        if (combatUnit->animateShot_view == CombatUnit::Left)
                        {
                            relativeShotStartPos = COMBATUNIT_SHOT_OFFSET;
                        }
                        else
                        {
                            vector2fl reversedShotOffset(COMBATUNIT_SHOT_OFFSET);
                            reversedShotOffset.y *= -1;
                            relativeShotStartPos = reversedShotOffset;
                        }
                        vector2fl rotated = relativeShotStartPos.rotated(combatUnit->angle_view);
                        vector2fl final = vector2fl(combatUnit->getPos()) + rotated;

                        sf::Color color;
                        float width;
                        int lifetime;
                        if (combatUnit->typechar() == TURRET_TYPECHAR)
                        {
                            color = sf::Color(200, 0, 200);
                            width = 5;
                            lifetime = 20;
                        }
                        else if (combatUnit->typechar() == FIGHTER_TYPECHAR)
                        {
                            color = sf::Color(255, 0, 0);
                            width = 1;
                            lifetime = 8;
                        }

                        boost::shared_ptr<LineParticle> line(new LineParticle(final, targetPos, color, width, lifetime));
                        ui->particles.addLineParticle(line);
                    }
                }
            }
        }
        for (unsigned int i=0; i<ui->selectedUnits.size(); i++)
        {
            float dashOffsetAbs = float(game->frame % 40) / 40;
            GH::DashedLineGroup dashedLinesOutflow(10, dashOffsetAbs);
            GH::DashedLineGroup dashedLinesInflow(10, -dashOffsetAbs);
            if (auto prime = boost::dynamic_pointer_cast<Prime, Entity>(ui->selectedUnits[i]))
            {
                // draw queue lines

                optional<vector2fp> lastToPoint;
                for (unsigned int j=0; j<prime->buildTargetQueue.size(); j++)
                {
                    if (auto entity = game->entities[prime->buildTargetQueue[j]])
                    {
                        auto fromPoint = lastToPoint ? *lastToPoint : prime->getPos();
                        auto toPoint = entity->getPos();
                        dashedLinesOutflow.pushLine(
                            gamePosToScreenPos(ui->camera, fromPoint),
                            gamePosToScreenPos(ui->camera, toPoint),
                            BUILD_JOB_LINE_COLOR
                        );
                        lastToPoint = toPoint;
                    }
                }
                lastToPoint = {};
                for (unsigned int j=0; j<prime->scavengeTargetQueue.size(); j++)
                {
                    if (auto point = prime->scavengeTargetQueue[j].getPointUnlessTargetDeleted(*game))
                    {
                        auto fromPoint = lastToPoint ? *lastToPoint : prime->getPos();
                        dashedLinesOutflow.pushLine(
                            gamePosToScreenPos(ui->camera, fromPoint),
                            gamePosToScreenPos(ui->camera, *point),
                            SCUTTLE_JOB_LINE_COLOR
                        );
                        lastToPoint = point;
                    }
                }
                if (prime->fundsSource && game->entities[*prime->fundsSource])
                {
                    dashedLinesInflow.pushLine(
                        gamePosToScreenPos(ui->camera, prime->getPos()),
                        gamePosToScreenPos(ui->camera, game->entities[*prime->fundsSource]->getPos()),
                        FUNDS_LINE_COLOR
                    );
                }
                if (prime->fundsDest && game->entities[*prime->fundsDest])
                {
                    dashedLinesOutflow.pushLine(
                        gamePosToScreenPos(ui->camera, prime->getPos()),
                        gamePosToScreenPos(ui->camera, game->entities[*prime->fundsDest]->getPos()),
                        FUNDS_LINE_COLOR
                    );
                }
            }
            if (auto gateway = boost::dynamic_pointer_cast<Gateway, Entity>(ui->selectedUnits[i]))
            {
                // draw queue lines
                for (unsigned int j = 0; j<gateway->buildTargetQueue.size(); j++)
                {
                    if (auto entity = game->entities[gateway->buildTargetQueue[j]])
                    {
                        dashedLinesOutflow.pushLine(
                            gamePosToScreenPos(ui->camera, gateway->getPos()),
                            gamePosToScreenPos(ui->camera, entity->getPos()),
                            BUILD_JOB_LINE_COLOR
                        );
                    }
                }
                for (unsigned int j = 0; j<gateway->scuttleTargetQueue.size(); j++)
                {
                    if (auto entity = game->entities[gateway->scuttleTargetQueue[j]])
                    {
                        dashedLinesInflow.pushLine(
                            gamePosToScreenPos(ui->camera, gateway->getPos()),
                            gamePosToScreenPos(ui->camera, entity->getPos()),
                            SCUTTLE_JOB_LINE_COLOR
                        );
                    }
                }
            }

            dashedLinesOutflow.render(window);
            dashedLinesInflow.render(window);

            drawSelectionCircleAroundEntity(window, ui->camera, ui->selectedUnits[i]);
        }

        if (!ui->cleanDrawEnabled)
        {
            drawUnitDroppableValues(window, game, ui, maybePlayerId, mainFont);
            drawUnitHealthBars(window, game, ui, maybePlayerId);
        }

        if (maybePlayerId)
        {
            uint8_t playerId = *maybePlayerId;
            coinsInt playerCredit = game->players[playerId].credit.getInt();

            sf::Color balanceTextColor =
                (playerCredit - lastPlayerCredit == 0 ? sf::Color::White :
                playerCredit > lastPlayerCredit ?      sf::Color::Green :
                                                        sf::Color::Red
                );
            drawAccountBalance(window, &game->players[playerId].credit, mainFont, balanceTextColor, sf::Vector2f(20, 20), drawWalletHints);

            lastPlayerCredit = playerCredit;
        }

        if (! ui->hideUX)
        {
        vector<sf::String> outputStrings;
        drawOutputStrings(window, outputStrings, mainFont);
        }

        bool playerOwnsUnits(false);
        for (unsigned int i=0; i<game->entities.size(); i++)
        {
            if (game->entities[i])
                if (getAllianceType(maybePlayerId, game->entities[i]) == Owned)
                {
                    playerOwnsUnits = true;
                    break;
                }
        }
        if (!ui->hideUX)
        {
        if (playerOwnsUnits)
            drawUnitHotkeyHelp(window, ui, mainFont);
        else
            drawSpawnBeaconHotkey(window, ui, mainFont);
        }

        if (ui->escapeTextCountdownOrNeg1 >= 0)
        {
            drawEscapeQuitText(window, (unsigned int)ui->escapeTextCountdownOrNeg1, ui->countdownToQuitOrNeg1, mainFont);
        }

        drawCursorOrSelectionBox(window, ui, maybePlayerId);
    }

    if (ui->showTutorial && tutorial && !tutorial->isFinished() && (!ui->hideUX))
    {
        displayTutorial(window, tutorial, game, ui, 500, tutorialFont);
    }

    window->display();
}

void cleanupGraphics(sf::RenderWindow* window)
{
    delete window;
}
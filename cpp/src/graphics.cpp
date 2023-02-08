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

sf::View uxView;
sf::View gameView;
sf::View screenView;

sf::Texture
    cmdAttackSource,
    cmdCollectSource,
    cmdScuttleSource,
    cmdInvestSource,
    cmdMoveSource,
    cmdStopSource,
    cmdWarpInSource,
    cmdWarpOutSource
;
sf::RenderTexture
    cmdBuildPrimeSource,
    cmdBuildFighterSource,
    cmdBuildGatewaySource,
    cmdBuildTurretSource
;

sf::Sprite
    cmdAttackIcon,
    cmdCollectIcon,
    cmdScuttleIcon,
    cmdInvestIcon,
    cmdMoveIcon,
    cmdStopIcon,
    cmdWarpInIcon,
    cmdWarpOutIcon,
    cmdBuildPrimeIcon,
    cmdBuildFighterIcon,
    cmdBuildGatewayIcon,
    cmdBuildTurretIcon
;

void loadFonts(sf::Font* mainFont, sf::Font* tutorialFont)
{
    if (!mainFont->loadFromFile("Andale_Mono.ttf"))
        throw runtime_error("Can't load main font");
    if (!tutorialFont->loadFromFile("NotoSansCJK-Regular.ttc"))
        throw runtime_error("Can't load tutorial font");
}

sf::RenderWindow* setupGraphics(bool fullscreen, bool smallScreen)
{
    loadKeyCommandIcons();

    // choose a good videomode
    sf::VideoMode chosenMode;
    if (smallScreen)
    {
        chosenMode = sf::VideoMode(800, 600, 24);
    }
    else if (!fullscreen)
    {
        chosenMode = sf::VideoMode(1800, 1000, 24);
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
            chosenMode = modes[0];
        }
        chosenMode.bitsPerPixel = 24;
    }

    auto flags =
        fullscreen ? sf::Style::Close | sf::Style::Fullscreen
                   : sf::Style::Close | sf::Style::Titlebar;

    sf::RenderWindow* window = new sf::RenderWindow(chosenMode, "Coinfight Client", flags);
    window->setKeyRepeatEnabled(false);

    // setup views
    screenView = window->getDefaultView();

    // as a quick fix for big resolutions, we double the UX size if the screen is large.    
    if (getScreenSize(window).x > 2000)
        uxView = sf::View(sf::FloatRect(sf::Vector2f(0, 0), toSFVecF(getScreenSize(window) / 2)));
    else
        uxView = screenView;

    return window;
}

void drawGameBackground(sf::RenderWindow *window)
{
    sf::Color backgroundColor(0, 0, 50);
    sf::Color circlesOutlineColor(60, 0, 150);

    float mapRadius(calculateMapRadius());

    sf::CircleShape biggestCircle(mapRadius, 300);
    biggestCircle.setFillColor(backgroundColor);
    biggestCircle.setOutlineColor(circlesOutlineColor);
    biggestCircle.setOutlineThickness(1);
    biggestCircle.setOrigin(mapRadius, mapRadius);
    biggestCircle.setPosition(sf::Vector2f(0,0));
    window->draw(biggestCircle);

    sf::Color backgroundSpokeEndColor(circlesOutlineColor);
    backgroundSpokeEndColor.a = 50;

    sf::CircleShape circle(1, 40);
    circle.setFillColor(sf::Color::Transparent);
    circle.setOutlineColor(circlesOutlineColor);
    circle.setOutlineThickness(1);
    circle.setPosition(sf::Vector2f(0,0));

    for (unsigned int i=0; i<NUM_MAP_RINGS; i++)
    {
        float radius(calculateMapRingRadius(i));
        float nextRadius = 10 * pow(2, i+2);

        // draw circle
        circle.setRadius(radius);
        circle.setPointCount(i * 20);
        circle.setOrigin(sf::Vector2f(radius, radius));

        window->draw(circle);

        if (i > 0)
        {
            unsigned int numSpokes = pow(2, i);

            sf::VertexArray lines(sf::PrimitiveType::Lines, numSpokes*2);
            for (unsigned int i=0; i<numSpokes; i++)
            {
                float angle = ((float)i / numSpokes) * 2 * M_PI;
                vector2fl from = composeVector2fl(angle, radius);
                vector2fl to = composeVector2fl(angle, nextRadius);

                lines[i*2].position = toSFVecF(from);
                lines[i*2+1].position = toSFVecF(to);
                lines[i*2].color = circlesOutlineColor;
                lines[i*2+1].color = backgroundSpokeEndColor;
            }

            window->draw(lines);
        }
    }
}

void drawEnergyLine(sf::RenderWindow *window, vector2fl from, vector2fl to, sf::Color color)
{
    vector2fl lineVec = (to - from);
    float length = lineVec.getMagnitude();

    // travel along length and draw segments with perturbed points

    int numNonEndPoints = length / ENERGY_LINE_SEGMENT_LENGTH; // i.e. if length is 1 and ENERGY(..) is 1.2, this would be zero because the only points would be the end points.

    sf::VertexArray lines(sf::PrimitiveType::LineStrip, numNonEndPoints + 2);

    lines[0].position = toSFVecF(from);
    lines[0].color = color;
    for (int i=0; i<numNonEndPoints; i++)
    {
        int vertexI = i + 1;
        float relativeLinePosition = ((float)vertexI / (numNonEndPoints + 1));
        vector2fl pointOnLine = from + (lineVec * relativeLinePosition);
        vector2fl perturbed = pointOnLine + randomVectorWithMagnitude(ENERGY_LINE_PERTURB_AMOUNT);

        lines[vertexI].position = toSFVecF(perturbed);
        lines[vertexI].color = color;
    }
    lines[numNonEndPoints + 1].position = toSFVecF(to);
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

        sf::VertexArray diamond(sf::PrimitiveType::TriangleFan, 4);
        diamond[0].position = sf::Vector2f(width/2, 0);
        diamond[1].position = sf::Vector2f(width/3, -height);
        diamond[2].position = sf::Vector2f(-width/3, -height);
        diamond[3].position = sf::Vector2f(-width/2, 0);
        diamond[0].color = goldBottomColor;
        diamond[1].color = goldTopColor;
        diamond[2].color = goldTopColor;
        diamond[3].color = goldBottomColor;

        sf::Transform transform;
        transform.translate(toSFVecF(drawPos + vector2fl(0, height/2)));

        window->draw(diamond, transform);
    }
}

const sf::Color UNIT_OUTLINE_COLOR(100, 100, 100);

void drawBeacon(sf::RenderTarget *renderTarget, vector2fl drawPos, sf::Color teamColor, unsigned int alpha)
{
    sf::Color teamColorFaded(teamColor.r, teamColor.g, teamColor.r, alpha);

    sf::Color fadedInnerOutlineColor(GATEWAY_INNEROUTLINE_COLOR.r, GATEWAY_INNEROUTLINE_COLOR.g, GATEWAY_INNEROUTLINE_COLOR.b, alpha);
    sf::RectangleShape innerRect(sf::Vector2f(10, 10));
    innerRect.setOrigin(sf::Vector2f(5, 5));
    innerRect.setFillColor(teamColorFaded);
    innerRect.setOutlineColor(teamColor);
    innerRect.setOutlineThickness(1.5);
    innerRect.setRotation(45);
    innerRect.setPosition(toSFVecF(drawPos));

    renderTarget->draw(innerRect);
}

void drawGateway(sf::RenderTarget *renderTarget, boost::shared_ptr<Gateway> gateway, vector2fl drawPos, float drawRotation, sf::Color teamColor, unsigned int alpha)
{
    // if there's a combat target in range, draw guns
    if (auto attackTarget = gateway->maybeAttackTarget)
    {
        // hacky solution: we only use the "unit's" transform for drawing the gun barrels.
        // This only moves the gun barrels and not the Gateway.
        
        sf::Transform transform;
        transform.translate(toSFVecF(drawPos));
        transform.rotate(radiansToDegrees(drawRotation));

        sf::VertexArray gunBarrelPoints(sf::PrimitiveType::Triangles, 3);
        gunBarrelPoints[0].position = sf::Vector2f(-8, 8);
        gunBarrelPoints[1].position = sf::Vector2f(-12, 16);
        gunBarrelPoints[2].position = toSFVecF(COMBATUNIT_SHOT_OFFSET);
        gunBarrelPoints[0].color = FIGHTER_BARREL_COLOR;
        gunBarrelPoints[1].color = FIGHTER_BARREL_COLOR;
        gunBarrelPoints[2].color = FIGHTER_BARREL_COLOR;
        renderTarget->draw(gunBarrelPoints, transform);

        gunBarrelPoints[0].position.y *= -1;
        gunBarrelPoints[1].position.y *= -1;
        gunBarrelPoints[2].position.y *= -1;
        renderTarget->draw(gunBarrelPoints, transform);
    }

    sf::Color fillColorFaded(GATEWAY_MAIN_COLOR.r, GATEWAY_MAIN_COLOR.g, GATEWAY_MAIN_COLOR.b, alpha);

    sf::CircleShape outerHex(15, 6);
    outerHex.setOrigin(sf::Vector2f(15, 15));
    outerHex.setFillColor(fillColorFaded);
    outerHex.setOutlineColor(UNIT_OUTLINE_COLOR);
    outerHex.setOutlineThickness(1);
    outerHex.setPosition(toSFVecF(drawPos));
    outerHex.setRotation(90);

    renderTarget->draw(outerHex);

    drawBeacon(renderTarget, drawPos, teamColor, alpha);
}

void drawPrime(sf::RenderTarget *renderTarget, boost::shared_ptr<Prime> prime, vector2fl pos, float drawRotation, unsigned int alpha)
{
    sf::Transform transform = sf::Transform();
    transform.translate(toSFVecF(pos));
    transform.rotate(radiansToDegrees(drawRotation));

    sf::Color teamColor = prime->getTeamColor();
    sf::Color mainPrimeColor(teamColor.r, teamColor.g, teamColor.b, alpha);

    float borderThickness = 2;
    float primeCavityRadius = static_cast<float>(PRIME_RADIUS) - borderThickness;

    sf::VertexArray wingPoints(sf::PrimitiveType::Triangles, 3);
    wingPoints[0].position = sf::Vector2f(0, static_cast<float>(PRIME_RADIUS));
    wingPoints[1].position = sf::Vector2f(-static_cast<float>(PRIME_RADIUS)*1.4, static_cast<float>(PRIME_RADIUS));
    wingPoints[2].position = toSFVecF(composeVector2fl(0.8 * M_PI, static_cast<float>(PRIME_RADIUS)));
    wingPoints[0].color = wingPoints[1].color = wingPoints[2].color = mainPrimeColor;
    renderTarget->draw(wingPoints, transform);

    wingPoints[0].position.y *= -1;
    wingPoints[1].position.y *= -1;
    wingPoints[2].position.y *= -1;
    renderTarget->draw(wingPoints, transform);

    sf::CircleShape structureOutline(primeCavityRadius);
    structureOutline.setOrigin(sf::Vector2f(primeCavityRadius, primeCavityRadius));
    structureOutline.setFillColor(sf::Color::Transparent);
    structureOutline.setOutlineColor(UNIT_OUTLINE_COLOR);
    structureOutline.setOutlineThickness(1);
    renderTarget->draw(structureOutline, transform);

    sf::CircleShape thickBorder(structureOutline);
    thickBorder.setOutlineColor(mainPrimeColor);
    thickBorder.setOutlineThickness(borderThickness);

    renderTarget->draw(thickBorder, transform);

    float heldGoldRatio = static_cast<float>(prime->getHeldGoldRatio());
    if (heldGoldRatio > 0)
    {
        float innerGoldRadius = 0.5 + (sqrt(heldGoldRatio) * (primeCavityRadius - 0.5));
        // radius is at least 0.5 (so any gold draws a circle of width 1) and scales up to primeCavityRadius

        sf::CircleShape heldGoldCircle(innerGoldRadius);
        heldGoldCircle.setOrigin(sf::Vector2f(innerGoldRadius, innerGoldRadius));
        heldGoldCircle.setPosition(toSFVecF(pos));
        heldGoldCircle.setFillColor(sf::Color::Yellow);

        renderTarget->draw(heldGoldCircle);
    }
}

void drawFighter(sf::RenderTarget *renderTarget, vector2fl drawPos, float rotation, sf::Color teamColor, unsigned int alpha)
{
    sf::Color fillColorFaded(teamColor.r, teamColor.g, teamColor.r, alpha);

    sf::ConvexShape oneSide;
    oneSide.setPointCount(3);

    oneSide.setFillColor(fillColorFaded);
    oneSide.setPosition(toSFVecF(drawPos));
    oneSide.setRotation(radiansToDegrees(rotation));

    sf::Vector2f front = sf::Vector2f(12, 0);
    sf::Vector2f back = sf::Vector2f(-4, 0);
    sf::Vector2f right = sf::Vector2f(-12, 16);
    sf::Vector2f left = sf::Vector2f(-12, -16);

    // draw gun barrels
    sf::Color fadedBarrelColor(FIGHTER_BARREL_COLOR.r, FIGHTER_BARREL_COLOR.g, FIGHTER_BARREL_COLOR.b, alpha);
    sf::VertexArray gunBarrelPoints(sf::PrimitiveType::Triangles, 3);
    gunBarrelPoints[0].position = sf::Vector2f(-8, 8);
    gunBarrelPoints[1].position = sf::Vector2f(-12, 16);
    gunBarrelPoints[2].position = toSFVecF(COMBATUNIT_SHOT_OFFSET);
    gunBarrelPoints[0].color = fadedBarrelColor;
    gunBarrelPoints[1].color = fadedBarrelColor;
    gunBarrelPoints[2].color = fadedBarrelColor;

    sf::Transform transform = sf::Transform();
    transform.translate(toSFVecF(drawPos));
    transform.rotate(radiansToDegrees(rotation));
    renderTarget->draw(gunBarrelPoints, transform);

    gunBarrelPoints[0].position.y *= -1;
    gunBarrelPoints[1].position.y *= -1;
    gunBarrelPoints[2].position.y *= -1;
    renderTarget->draw(gunBarrelPoints, transform);

    // draw two triangles
    oneSide.setPoint(1, front);
    oneSide.setPoint(0, back);
    oneSide.setPoint(2, right);
    renderTarget->draw(oneSide);
    oneSide.setPoint(2, left);
    renderTarget->draw(oneSide);

    // draw outline
    sf::VertexArray lines(sf::PrimitiveType::LineStrip, 5);
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
    transform.translate(toSFVecF(drawPos));
    transform.rotate(radiansToDegrees(rotation));
    renderTarget->draw(lines, transform);
}

void drawTurret(sf::RenderTarget *renderTarget, vector2fl drawPos, float rotation, sf::Color teamColor, unsigned int alpha)
{
    sf::Color fillColorFaded(TURRET_MAIN_COLOR.r,TURRET_MAIN_COLOR.g, TURRET_MAIN_COLOR.b, alpha);

    int halfWidth = int(TURRET_RADIUS * fixed32(0.85));
    int width = halfWidth * 2;

    sf::RectangleShape box(sf::Vector2f(width, width));
    box.setOrigin(sf::Vector2f(halfWidth, halfWidth));
    box.setFillColor(fillColorFaded);
    box.setOutlineColor(UNIT_OUTLINE_COLOR);
    box.setOutlineThickness(1);
    box.setPosition(toSFVecF(drawPos));

    renderTarget->draw(box);

    drawFighter(renderTarget, drawPos, rotation, teamColor, alpha);
}

void drawUnit(sf::RenderWindow *window, boost::shared_ptr<Unit> unit, vector2fl drawPos)
{
    sf::Color teamColor = unit->getTeamColor(); // may be modified later if unit is not yet active
    float drawRotation = unit->angle_view;

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
        drawGateway(window, gateway, drawPos, drawRotation, teamColor, fadedAlpha);
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

void drawRadius(sf::RenderWindow* window, vector2fl drawPos, float radius, sf::Color color)
{
    sf::CircleShape circle(radius, 50);
    circle.setPosition(drawPos.x, drawPos.y);
    circle.setOutlineColor(color);
    circle.setOutlineThickness(1);
    circle.setFillColor(sf::Color::Transparent);
    circle.setOrigin(sf::Vector2f(radius, radius));

    window->draw(circle);
}

void drawUnitRadii(sf::RenderWindow* window, boost::shared_ptr<Unit> unit, vector2fl drawPos)
{
    if (auto combatUnit = boost::dynamic_pointer_cast<CombatUnit, Unit>(unit))
    {
        drawRadius(window, drawPos, float(combatUnit->getShotRange()), sf::Color(255, 0, 0, 100));
    }
    if (auto gateway = boost::dynamic_pointer_cast<Gateway, Unit>(unit))
    {
        drawRadius(window, drawPos, float(GATEWAY_RANGE), sf::Color(255, 255, 0, 100));
    }
}

void drawEntity(sf::RenderWindow *window, boost::shared_ptr<Entity> entity, vector2fl drawPos)
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

// vector2i scaledDownGamePosWithZCorrection(vector2fl gamePos, float scaleDownFactor)
// {
//     return vector2i(
//         (gamePos.x > 0 ?
//             (int)(gamePos.x / scaleDownFactor)
//             : (int)((gamePos.x / scaleDownFactor) - 1)
//         ), // otherwise everything between -1 and 1 shows up at col (and below, row) 0
//         (gamePos.y > 0 ?
//             (int)(gamePos.y / scaleDownFactor)
//             : (int)((gamePos.y / scaleDownFactor) - 1)
//         )
//     );
// }

// void drawEntitySymbolOnMinimap(sf::RenderWindow *window, boost::shared_ptr<Entity> entity, optional<uint8_t> maybeViewingPlayerId, float zoomOutFactor)
// {
//     vector2i minimapPos = scaledDownGamePosWithZCorrection(vector2fl(entity->getPos()), zoomOutFactor);
//     minimapPos.y *= -1;

//     sf::RectangleShape pixel(sf::Vector2f(1,1));

//     pixel.setOrigin(sf::Vector2f(0.5, 0.5));
//     pixel.setPosition(sf::Vector2f(toSFVecF(minimapPos + screenCenter)));
//     pixel.setFillColor(entity->getTeamOrPrimaryColor());

//     window->draw(pixel);
// }

void displayAccountBalance(sf::RenderWindow *window, Coins *playerBalance, sf::Font* font, sf::Color balanceTextColor, sf::Vector2f upperLeft, bool displayWalletHints)
{
    if (displayWalletHints)
    {
        vector<sf::Text> hints
            {sf::Text(sf::String("This can be spent or captured via Gateways"), *font, 16),
            sf::Text(sf::String("and withdraws/deposits into xDai."), *font, 16)
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

    sf::Text title(sf::String("Wallet"), *font, 24);

    sf::Text balance(sf::String(playerBalance->getDollarString()), *font, 30);
    balance.setFillColor(balanceTextColor);

    sf::Transform transform;
    transform.translate(upperLeft);

    window->draw(title, transform);

    transform.translate(sf::Vector2f(0, title.getLocalBounds().height + textSpacing));
    window->draw(balance, transform);
}

void displayDepositNeededMsg(sf::RenderWindow* window, sf::Font* font, sf::Vector2f upperLeft)
{
    string msg = "To participate in the round, you'll need to either:\n* deposit at least $4.50 at coinfight.io to afford your first Gateway and Prime\n* receive some units as a gift from another player.";
    sf::Text text(sf::String(msg), *font, 16);

    text.setFillColor(sf::Color::White);
    text.setPosition(upperLeft);

    window->draw(text);
}

void displayOutputStrings(sf::RenderWindow *window, vector<sf::String> strings, sf::Font* font)
{
    for (unsigned int i=0; i<strings.size(); i++)
    {
        sf::Text text(strings[i], *font, 16);
        text.setFillColor(sf::Color(150, 150, 150));

        float width = text.getLocalBounds().width;
        int x, y;
        x = getCurrentViewSize(window).x - width - 10;
        y = 6 + i * 20;
        text.setPosition(sf::Vector2f(x, y));

        window->draw(text);
    }
}

void drawCircleAround(sf::RenderWindow *window, vector2fl pos, unsigned int radius, unsigned int thickness, sf::Color color)
{
    sf::Transform mouseTransform;
    mouseTransform.translate(toSFVecF(pos));

    sf::CircleShape circle(radius);
    circle.setOrigin(sf::Vector2f(radius, radius));
    circle.setOutlineColor(color);
    circle.setFillColor(sf::Color::Transparent);
    circle.setOutlineThickness(thickness);

    window->draw(circle, mouseTransform);
}

void displayNormalCursor(sf::RenderWindow *window)
{
    window->setMouseCursorVisible(true);
}

void displayTargetCursor(sf::RenderWindow *window, vector2i mousePos, sf::Color color)
{
    window->setMouseCursorVisible(false);

    sf::VertexArray lines(sf::PrimitiveType::Lines, 2);
    lines[0].position = sf::Vector2f(CURSOR_SIZE/2, CURSOR_SIZE/2);
    lines[1].position = sf::Vector2f(4, 4);
    lines[0].color = color;
    lines[1].color = color;

    sf::Transform transform;
    transform.translate(toSFVecF(mousePos));
    window->draw(lines, transform);
    transform.rotate(90);
    window->draw(lines, transform);
    transform.rotate(90);
    window->draw(lines, transform);
    transform.rotate(90);
    window->draw(lines, transform);

    sf::RectangleShape rect(sf::Vector2f(2,2));
    rect.setPosition(sf::Vector2f(-1, -1));
    rect.setFillColor(color);

    transform = sf::Transform();
    transform.translate(toSFVecF(mousePos));
    window->draw(rect, transform);
}
void displayBracketsCursor(sf::RenderWindow *window, vector2i mousePos, sf::Color color)
{
    window->setMouseCursorVisible(false);

    sf::VertexArray lines(sf::PrimitiveType::LineStrip, 3);
    lines[0].position = sf::Vector2f(CURSOR_SIZE/2 - 6, CURSOR_SIZE/2);
    lines[1].position = sf::Vector2f(CURSOR_SIZE/2, CURSOR_SIZE/2);
    lines[2].position = sf::Vector2f(CURSOR_SIZE/2, CURSOR_SIZE/2 - 6);
    lines[0].color = color;
    lines[1].color = color;
    lines[2].color = color;

    sf::Transform transform;
    transform.translate(toSFVecF(mousePos));
    window->draw(lines, transform);
    transform.rotate(90);
    window->draw(lines, transform);
    transform.rotate(90);
    window->draw(lines, transform);
    transform.rotate(90);
    window->draw(lines, transform);
}

void displaySelectableCursor(sf::RenderWindow *window, vector2i mousePos)
{
    displayBracketsCursor(window, mousePos, sf::Color::Green);
}

void drawGhostBuilding(sf::RenderWindow *window, const GameUI* ui, vector2i mousePos)
{
    window->setMouseCursorVisible(false);

    drawEntity(window, ui->ghostBuilding, mousePos);
}

void displaySelectionBox(sf::RenderWindow *window, vector2i p1, vector2i p2)
{
    int rectLeft = min(p1.x, p2.x);
    int rectRight = max(p1.x, p2.x);
    int rectBottom = max(p1.y, p2.y);
    int rectTop = min(p1.y, p2.y);
    int width = rectRight - rectLeft;
    int height = rectBottom - rectTop;

    sf::RectangleShape rect(sf::Vector2f(width, height));
    rect.setPosition(sf::Vector2f(rectLeft, rectTop));
    rect.setOutlineColor(sf::Color::Green);
    rect.setOutlineThickness(1);
    rect.setFillColor(sf::Color::Transparent);

    window->draw(rect);
}

void displayCursorOrSelectionBox(sf::RenderWindow *window, GameUI* ui, optional<uint8_t> maybePlayerId)
{
    vector2i mousePos = vector2i(sf::Mouse::getPosition(*window).x, sf::Mouse::getPosition(*window).y);

    if (ui->maybeSelectionBoxStart)
    {
        displaySelectionBox(window, *ui->maybeSelectionBoxStart, mousePos);
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
                        displayBracketsCursor(window, mousePos, sf::Color::Red);
                    }
                    else
                    {
                        displaySelectableCursor(window, mousePos);
                    }
                }
                else
                {
                    displayNormalCursor(window);
                }
                break;
            case GameUI::SpawnBeacon:
                displayTargetCursor(window, mousePos, sf::Color::Yellow);
                drawUnitRadii(window, ui->ghostBuilding, mousePos);
                break;
            case GameUI::AttackScuttle:
                if (ui->mouseoverEntity)
                {
                    displayBracketsCursor(window, mousePos, sf::Color::Red);
                }
                else
                {
                    displayTargetCursor(window, mousePos, sf::Color::Red);
                }
                break;
            case GameUI::Deposit:
                if (ui->mouseoverEntity)
                {
                    displayBracketsCursor(window, mousePos, sf::Color::Blue);
                }
                else
                {
                    displayTargetCursor(window, mousePos, sf::Color::Blue);
                }
                break;
            case GameUI::Fetch:
                if (ui->mouseoverEntity)
                {
                    if (getAllianceType(maybePlayerId, ui->mouseoverEntity) == Owned || ui->mouseoverEntity->typechar() == GOLDPILE_TYPECHAR)
                    {
                        displayBracketsCursor(window, mousePos, sf::Color::Yellow);
                    }
                    else
                    {
                        displayBracketsCursor(window, mousePos, sf::Color(100, 100, 100));
                    }
                }
                else
                {
                    if (filterForTypeKeepContainer<Prime, Unit>(ui->selectedUnits).size() > 0)
                    {
                        displayTargetCursor(window, mousePos, sf::Color::Yellow);
                    }
                    else
                    {
                        displayBracketsCursor(window, mousePos, sf::Color(100, 100, 100));
                    }
                }
                break;
            case GameUI::Build:
                drawGhostBuilding(window, ui, mousePos);
                drawUnitRadii(window, ui->ghostBuilding, mousePos);
                break;
        }
    }
}

void drawSelectionCircleAroundEntity(sf::RenderWindow *window, boost::shared_ptr<Entity> entity)
{
    drawCircleAround(window, vector2fl(entity->getPos()), 15, 1, sf::Color::Green);
}

void drawUnitDroppableValues(sf::RenderWindow *window, Game *game, GameUI* ui, optional<uint8_t> maybePlayerId, sf::Font* font)
{
    for (unsigned int i=0; i<game->entities.size(); i++)
    {
        if (!game->entities[i])
            continue;
        Coins *displayAboveCoins = NULL;
        // Coins *displayBelowCoins = NULL;

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
            sf::Text aboveText(displayAboveCoins->getDollarString(), *font, 16);
            sf::FloatRect textRec = aboveText.getLocalBounds();

            vector2fl textPos(entityPos + vector2fp(fixed32(0), fixed32(-30)));

            aboveText.setFillColor(topTextColor);
            aboveText.setOrigin(textRec.width / 2.f, textRec.height / 2.f);
            aboveText.setPosition(toSFVecF(textPos));

            sf::RectangleShape drawRect(sf::Vector2f(textRec.width + 4, textRec.height + 4));
            drawRect.setOrigin((textRec.width / 2.f) + 2, (textRec.height / 2.f) + 2);
            drawRect.setPosition(toSFVecF(textPos + vector2fl(0, 3)));
            drawRect.setFillColor(sf::Color(0, 0, 0, 150));

            window->draw(drawRect);
            window->draw(aboveText);
        }
        // the below was not being used - so I commented it out for now.
        // To use this you'll have to invert some of the y vals (this code is from before sf::View was used)
        // 
        // if (displayBelowCoins && displayBelowCoins->getInt() > 0)
        // {
        //     sf::Text belowText(displayBelowCoins->getDollarString(), font, 16);
        //     sf::FloatRect textRec = belowText.getLocalBounds();

        //     vector2fl textPos(entityPos + vector2fp(fixed32(0), fixed32(20)));

        //     belowText.setFillColor(sf::Color(200, 200, 255));
        //     belowText.setOrigin(textRec.width / 2.f, textRec.height / 2.f);
        //     belowText.setPosition(toSFVecF(textPos));

        //     sf::RectangleShape drawRect(sf::Vector2f(textRec.width + 3, textRec.height + 3));
        //     drawRect.setOrigin(textRec.width / 2.f, textRec.height / 2.f);
        //     drawRect.setPosition(toSFVecF(textPos + vector2fl(0, -3)));
        //     drawRect.setFillColor(sf::Color(0, 0, 0, 150));

        //     window->draw(drawRect);
        //     window->draw(belowText);
        // }
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

            vector2fp healthBarPos = unit->getPos() + vector2fp(fixed32(0), unit->getRadius() + 8);

            float healthBarLength = static_cast<float>(unit->getRadius()) * 2;

            sf::RectangleShape healthBar(sf::Vector2f(healthBarLength, 6)); // will be used to draw both outline and fill
            healthBar.setOrigin(healthBar.getLocalBounds().width / 2.f, healthBar.getLocalBounds().height / 2.f);
            healthBar.setPosition(toSFVecF(healthBarPos));

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

void drawAppropriateRadii(sf::RenderWindow* window, Game* game, GameUI* ui)
{
    if (ui->displayAllRadii)
    {
        for (unsigned int i=0; i<game->entities.size(); i++)
        {
            if (auto unit = boost::dynamic_pointer_cast<Unit, Entity>(game->entities[i]))
            {
                drawUnitRadii(window, unit, vector2fl(unit->getPos()));
            }
        }
    }
    else if (ui->mouseoverEntity)
    {
        if (auto unit = boost::dynamic_pointer_cast<Unit, Entity>(ui->mouseoverEntity))
        {
            drawUnitRadii(window, unit, vector2fl(unit->getPos()));
        }
    }
}

// void drawArrow(sf::RenderWindow* window, GameUI* ui, vector2fp drawPos, bool pointingUp, sf::Color color)
// {
//     sf::ConvexShape arrowPoint;
//     arrowPoint.setPointCount(3);
//     arrowPoint.setPoint(0, sf::Vector2f(0, -4));
//     arrowPoint.setPoint(1, sf::Vector2f(-5, 4));
//     arrowPoint.setPoint(2, sf::Vector2f(5, 4));

//     arrowPoint.setFillColor(color);
//     arrowPoint.setPosition(toSFVecF(drawPos));

//     if (!pointingUp)
//     {
//         arrowPoint.setRotation(180);
//     }

//     window->draw(arrowPoint);
// }

const int HOTKEY_BOX_WIDTH = 60;
const int HOTKEY_BOX_SPACING = 10;
const int HOTKEY_BOTTOMROW_INDENT = 18;

void displayTutorial(sf::RenderWindow *window, Tutorial* tutorial, Game* game, GameUI* ui, int boxWidth, sf::Font* font)
{
    sf::Transform transform;
    transform.translate(
        sf::Vector2f(
            getCurrentViewSize(window).x - boxWidth - 10,
            10
        )
    );

    vector<string> preBarTextBlocks = get<0>(tutorial->currentStep()->getText(game, ui));
    vector<string> postBarTextBlocks = get<1>(tutorial->currentStep()->getText(game, ui));

    for (unsigned int i=0; i<preBarTextBlocks.size(); i++)
    {
        GH::wrapAndRenderTextWithTransform(window, preBarTextBlocks[i], font, 13, sf::Color::White, boxWidth, &transform);
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
                GH::wrapAndRenderTextWithTransform(window, postBarTextBlocks[i], font, 13, sf::Color::White, boxWidth, &transform);
            }
        }
    }
    
    if (tutorial->currentStep()->isReadyToFinish(game, ui) && tutorial->currentStep()->waitForEnter)
    {
        GH::wrapAndRenderTextWithTransform(window, "Press enter to continue", font, 16, sf::Color::White, boxWidth, &transform);
    }
}

void drawUnits(sf::RenderWindow* window, Game* game, GameUI* ui)
{
    for (unsigned int i = 0; i < game->entities.size(); i++)
    {
        if (game->entities[i])
        {
            vector2fl pos (game->entities[i]->getPos());
            drawEntity(window, game->entities[i], pos);
        }
    }
}

void drawBuildRelatedEffects(sf::RenderWindow* window, Game* game, GameUI* ui)
{
    for (unsigned int i = 0; i < game->entities.size(); i++)
    {
        if (game->entities[i])
        {
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
                            drawEnergyLine(window, vector2fl(flowFromEntity->getPos()), vector2fl(prime->getPos()), sf::Color::Red);
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
                            drawEnergyLine(window, vector2fl(flowToEntity->getPos()), vector2fl(prime->getPos()), sf::Color::Blue);
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
                            drawEnergyLine(window, vector2fl(flowFromEntity->getPos()), vector2fl(gateway->getPos()), sf::Color::Red);
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
                            drawEnergyLine(window, vector2fl(flowToEntity->getPos()), vector2fl(gateway->getPos()), sf::Color::Blue);
                        }
                    }
                }
            }
        }
    }
}

void drawCombatUnitShots(sf::RenderWindow* window, Game* game, GameUI* ui)
{
    for (unsigned int i=0; i<game->entities.size(); i++)
    {
        if (game->entities[i])
        {
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
                    if (combatUnit->typechar() == TURRET_TYPECHAR || combatUnit->typechar() == GATEWAY_TYPECHAR)
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
                    else
                    {
                        cout << "Error: I don't know how to draw that CombatUnit shot!" << endl;
                    }

                    boost::shared_ptr<LineParticle> line(new LineParticle(final, targetPos, color, width, lifetime));
                    ui->particles.addLineParticle(line);
                }
            }
        }
    }
}

void drawGame(sf::RenderWindow* window, Game* game, GameUI* ui)
{
    drawGameBackground(window);

    drawBuildRelatedEffects(window, game, ui);
    drawCombatUnitShots(window, game, ui);
    drawUnits(window, game, ui);

    ui->particles.drawParticles(window);
    ui->particles.iterateParticles(*game);
}

void drawQueueLinesForSelectedUnits(sf::RenderWindow* window, Game* game, GameUI* ui)
{
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
                    dashedLinesOutflow.pushLine(vector2fl(fromPoint), vector2fl(toPoint), BUILD_JOB_LINE_COLOR);
                    lastToPoint = toPoint;
                }
            }
            lastToPoint = {};
            for (unsigned int j=0; j<prime->scavengeTargetQueue.size(); j++)
            {
                if (auto point = prime->scavengeTargetQueue[j].getPointUnlessTargetDeleted(*game))
                {
                    auto fromPoint = lastToPoint ? *lastToPoint : prime->getPos();
                    dashedLinesOutflow.pushLine(vector2fl(fromPoint), vector2fl(*point), SCUTTLE_JOB_LINE_COLOR);
                    lastToPoint = point;
                }
            }
            if (prime->fundsSource && game->entities[*prime->fundsSource])
            {
                dashedLinesInflow.pushLine(vector2fl(prime->getPos()), vector2fl(game->entities[*prime->fundsSource]->getPos()), FUNDS_LINE_COLOR);
            }
            if (prime->fundsDest && game->entities[*prime->fundsDest])
            {
                dashedLinesOutflow.pushLine(vector2fl(prime->getPos()), vector2fl(game->entities[*prime->fundsDest]->getPos()), FUNDS_LINE_COLOR);
            }
        }
        if (auto gateway = boost::dynamic_pointer_cast<Gateway, Entity>(ui->selectedUnits[i]))
        {
            // draw queue lines
            for (unsigned int j = 0; j<gateway->buildTargetQueue.size(); j++)
            {
                if (auto entity = game->entities[gateway->buildTargetQueue[j]])
                {
                    dashedLinesOutflow.pushLine(vector2fl(gateway->getPos()), vector2fl(entity->getPos()), BUILD_JOB_LINE_COLOR);
                }
            }
            for (unsigned int j = 0; j<gateway->scuttleTargetQueue.size(); j++)
            {
                if (auto entity = game->entities[gateway->scuttleTargetQueue[j]])
                {
                    dashedLinesInflow.pushLine(vector2fl(gateway->getPos()), vector2fl(entity->getPos()), SCUTTLE_JOB_LINE_COLOR);
                }
            }
        }

        dashedLinesOutflow.render(window);
        dashedLinesInflow.render(window);
    }
}

void drawSelectedUnitExtras(sf::RenderWindow* window, Game* game, GameUI* ui)
{
    drawQueueLinesForSelectedUnits(window, game, ui);

    for (unsigned int i=0; i<ui->selectedUnits.size(); i++)
    {
        drawSelectionCircleAroundEntity(window, ui->selectedUnits[i]);
    }
}

void drawGameOverlay(sf::RenderWindow* window, Game* game, GameUI* ui, optional<uint8_t> maybePlayerId, sf::Font* fwFont)
{
    if (!ui->cleanDrawEnabled)
    {
        drawUnitDroppableValues(window, game, ui, maybePlayerId, fwFont);
        drawUnitHealthBars(window, game, ui, maybePlayerId);
        drawAppropriateRadii(window, game, ui);
    }

    if (maybePlayerId)
    {
        drawSelectedUnitExtras(window, game, ui);
    }
}

coinsInt lastPlayerCredit = 0;
void displayWalletBalanceOrDepositNeededMsg(sf::RenderWindow* window, Game* game, GameUI* ui, optional<uint8_t> maybePlayerId, sf::Font* fwFont, sf::Font* humanFont, bool displayWalletHints)
{
    // depending on how maybePlayerAddress and maybePlayerId are set,
    // we might draw a balance or a message about coinfight.io.
    if (maybePlayerId) // player has logged in and is registered in the game (triggered by deposit)
    {
        uint8_t playerId = *maybePlayerId;
        coinsInt playerCredit = game->players[playerId].credit.getInt();

        sf::Color balanceTextColor =
            (playerCredit - lastPlayerCredit == 0 ? sf::Color::White :
             playerCredit > lastPlayerCredit ?      sf::Color::Green :
                                                    sf::Color::Red
            );
        displayAccountBalance(window, &game->players[playerId].credit, fwFont, balanceTextColor, sf::Vector2f(20, 20), displayWalletHints);

        lastPlayerCredit = playerCredit;
    }
    else // player has an address but no ID. To become a registered player in the game they need to deposit
    {
        displayDepositNeededMsg(window, humanFont, sf::Vector2f(20, 20));
    }
}

void displayKeyButtonHint(sf::RenderWindow* window, vector2i upperLeft, KeyButtonHintInfo hintInfo, sf::Font* fwFont, sf::Font* humanFont)
{
    sf::RectangleShape borderBox(toSFVecF(KEYBUTTONHINT_SIZE));
    borderBox.setPosition(toSFVecF(upperLeft));
    borderBox.setFillColor(sf::Color::Black);
    borderBox.setOutlineThickness(1);
    borderBox.setOutlineColor(UX_BOX_BORDER_COLOR);
    
    window->draw(borderBox);

    vector2i drawUpperLeft = upperLeft + UX_BOX_PADDING;
    vector2i drawAreaSize = KEYBUTTONHINT_SIZE - (UX_BOX_PADDING * 2);

    // title text
    string titleString = hintInfo.name + " (" + string(1, hintInfo.hotkeyChar) + ")";
    sf::Text titleText(titleString, *humanFont, 20);
    titleText.setPosition(toSFVecF(drawUpperLeft));
    titleText.setFillColor(sf::Color::White);
    window->draw(titleText);

    if (auto cost = hintInfo.maybeCost)
    {
        string costString = coinsIntToDollarString(*cost);
        sf::Text costText(costString, *fwFont, 18);
        int xOffset = drawAreaSize.x - costText.getLocalBounds().width;
        costText.setPosition(toSFVecF(drawUpperLeft + vector2i(xOffset, 0)));
        costText.setFillColor(sf::Color(255, 255, 0));
        window->draw(costText);
    }

    int yOffset = titleText.getLocalBounds().height + 20;
    int xOffset = 10;

    int textHeight = GH::wrapAndRenderTextAtPos(window, hintInfo.description, humanFont, 16, sf::Color::White, drawAreaSize.x, drawUpperLeft + vector2i(xOffset, yOffset));
    yOffset += textHeight + 20;

    sf::RectangleShape bulletPointRect(sf::Vector2f(5, 5));
    for (unsigned int i=0; i<hintInfo.bulletPoints.size(); i++)
    {
        vector2i itemDrawOffset(xOffset, yOffset);
        string lineText = hintInfo.bulletPoints[i];

        bulletPointRect.setPosition(toSFVecF(drawUpperLeft + itemDrawOffset + vector2i(0, 5)));
        bulletPointRect.setFillColor(sf::Color::White);

        window->draw(bulletPointRect);

        vector2i textDrawOffset = itemDrawOffset + vector2i(16, 0);
        int availableWidth = drawAreaSize.x - textDrawOffset.x;
        textHeight = GH::wrapAndRenderTextAtPos(window, lineText, humanFont, 12, sf::Color::White, availableWidth, drawUpperLeft + textDrawOffset);

        yOffset += textHeight + 16;
    }
}

void displayGameHUD(sf::RenderWindow* window, Game* game, GameUI* ui, optional<uint8_t> maybePlayerId, sf::Font* fwFont, sf::Font* humanFont, bool displayWalletHints)
{
    displayWalletBalanceOrDepositNeededMsg(window, game, ui, maybePlayerId, fwFont, humanFont, displayWalletHints);

    if (! ui->hideUX)
    {
        vector<sf::String> outputStrings;
        displayOutputStrings(window, outputStrings, fwFont);
    }

    ui->keyButtonBox.draw(window);
    ui->unitInfoBox.draw(window);
    if (auto hintToDisplay = ui->keyButtonBox.getMouseoverHintInfo())
    {
        displayKeyButtonHint(window, vector2i(ui->keyButtonBox.upperLeft.x + KEYBUTTONBOX_SIZE.x + 2, getCurrentViewSize(window).y - KEYBUTTONHINT_SIZE.y), *hintToDisplay, fwFont, humanFont);
    }
}

void drawSearchGridOverlay(sf::RenderWindow* window, Game* game)
{
    sf::RectangleShape cellBox(sf::Vector2f(SEARCH_GRID_CELL_WIDTH, SEARCH_GRID_CELL_WIDTH));
    for (unsigned int x=0; x < game->searchGrid.getNumCellRows(); x++)
    {
        for (unsigned int y=0; y < game->searchGrid.getNumCellRows(); y++)
        {
            cellBox.setFillColor(
                ((x + y) % 2 == 0) ?
                sf::Color(0, 255, 0, 20) :
                sf::Color(0, 0, 255, 20)
            );
            vector2fl boxPos = vector2fl(game->searchGrid.getOriginInGameSpace()) + (vector2fl(x, y) * SEARCH_GRID_CELL_WIDTH);
            cellBox.setPosition(toSFVecF(boxPos));
            window->draw(cellBox);
        }
    }
}

void display(sf::RenderWindow *window, Game *game, GameUI* ui, optional<Address> maybePlayerAddress, Tutorial* tutorial, sf::Font* fwFont, sf::Font* humanFont, bool displayWalletHints)
{
    gameView = ui->cameraView;

    window->clear();

    optional<uint8_t> maybePlayerId;
    if (maybePlayerAddress)
    {
        maybePlayerId = game->playerAddressToMaybeId(*maybePlayerAddress);
    }

    if (ui->minimapEnabled)
    {
        // displayMinimap(window, game, maybePlayerId, screenDimensions);
    }
    else
    {
        window->setView(gameView);
        drawGame(window, game, ui);
        drawGameOverlay(window, game, ui, maybePlayerId, fwFont);
        // drawSearchGridOverlay(window, game);
        window->setView(screenView);
        displayCursorOrSelectionBox(window, ui, maybePlayerId);
        window->setView(uxView);
        displayGameHUD(window, game, ui, maybePlayerId, fwFont, humanFont, displayWalletHints);
    }
    if (ui->showTutorial && tutorial && !tutorial->isFinished() && (!ui->hideUX))
    {
        window->setView(uxView);
        displayTutorial(window, tutorial, game, ui, 500, humanFont);
    }

    window->setView(screenView);
    if (ui->inGameMenu)
    {
        ui->inGameMenu->draw(window);
    }
    if (ui->giftUnitsWindow)
    {
        ui->giftUnitsWindow->draw(window);
    }

    window->display();
    
    window->setView(gameView);
}

void cleanupGraphics(sf::RenderWindow* window)
{
    delete window;
}

const vector<string> titleTextSrcLines = 
{
    " ___    ___    _    _  _    ___    _    ___    _ _    ___ ",
    "|  _}  |   |  | |  | \\| |  | __}  | |  |  _}  | | |  |_ _|",
    "| |_   | | |  | |  |  \\ |  | __}  | |  | | |  | - |   | |  ",
    "|___}  |___|  |_|  |_|\\_|  |_|    |_|  |___|  |_|_|   |_| "
     // note the "non-rendered" double backslashes here
};

void displayTitle(sf::RenderWindow* window, sf::Font* font)
{
    stringstream srcTextSS;
    for (unsigned int i=0; i<titleTextSrcLines.size(); i++)
    {
        if (i!=0)
        {
            srcTextSS << "\n";
        }
        srcTextSS << titleTextSrcLines[i];
    }
    string srcText = srcTextSS.str();

    sf::Text rendered(sf::String(srcText), *font, 20);

    unsigned int width = rendered.getLocalBounds().width;

    vector2i center = getScreenSize(window) / 2;

    vector2i drawPos(
        (center.x - (width / 2)),
        TITLE_POS_Y
    );
    rendered.setPosition(toSFVecF(drawPos));

    window->draw(rendered);
}

void loadKeyCommandIcons()
{
    if (!cmdAttackSource.loadFromFile("cmd-attack.png"))
    {
        throw runtime_error("Can't load icon png");
    }
    cmdAttackIcon.setTexture(cmdAttackSource);
    cmdAttackIcon.setScale(KEYBUTTON_ICON_SCALE, KEYBUTTON_ICON_SCALE);

    if (!cmdCollectSource.loadFromFile("cmd-collect.png"))
    {
        throw runtime_error("Can't load icon png");
    }
    cmdCollectIcon.setTexture(cmdCollectSource);
    cmdCollectIcon.setScale(KEYBUTTON_ICON_SCALE, KEYBUTTON_ICON_SCALE);

    if (!cmdScuttleSource.loadFromFile("cmd-scuttle.png"))
    {
        throw runtime_error("Can't load icon png");
    }
    cmdScuttleIcon.setTexture(cmdScuttleSource);
    cmdScuttleIcon.setScale(KEYBUTTON_ICON_SCALE, KEYBUTTON_ICON_SCALE);

    if (!cmdInvestSource.loadFromFile("cmd-invest.png"))
    {
        throw runtime_error("Can't load icon png");
    }
    cmdInvestIcon.setTexture(cmdInvestSource);
    cmdInvestIcon.setScale(KEYBUTTON_ICON_SCALE, KEYBUTTON_ICON_SCALE);

    if (!cmdMoveSource.loadFromFile("cmd-move.png"))
    {
        throw runtime_error("Can't load icon png");
    }
    cmdMoveIcon.setTexture(cmdMoveSource);
    cmdMoveIcon.setScale(KEYBUTTON_ICON_SCALE, KEYBUTTON_ICON_SCALE);

    if (!cmdStopSource.loadFromFile("cmd-stop.png"))
    {
        throw runtime_error("Can't load icon png");
    }
    cmdStopIcon.setTexture(cmdStopSource);
    cmdStopIcon.setScale(KEYBUTTON_ICON_SCALE, KEYBUTTON_ICON_SCALE);

    if (!cmdWarpInSource.loadFromFile("cmd-warp-in.png"))
    {
        throw runtime_error("Can't load icon png");
    }
    cmdWarpInIcon.setTexture(cmdWarpInSource);
    cmdWarpInIcon.setScale(KEYBUTTON_ICON_SCALE, KEYBUTTON_ICON_SCALE);

    if (!cmdWarpOutSource.loadFromFile("cmd-warp-out.png"))
    {
        throw runtime_error("Can't load icon png");
    }
    cmdWarpOutIcon.setTexture(cmdWarpOutSource);
    cmdWarpOutIcon.setScale(KEYBUTTON_ICON_SCALE, KEYBUTTON_ICON_SCALE);


    // for the build commands, we draw them to textures
    vector2i buttonDrawSize = KEYBUTTON_SIZE - KEYBUTTON_PADDING;

    if (!cmdBuildPrimeSource.create(buttonDrawSize.x, buttonDrawSize.y))
    {
        throw runtime_error("Can't create draw texture.");
    }
    boost::shared_ptr<Prime> tempPrime(new Prime(-1, vector2fp::zero));
    drawPrime(&cmdBuildPrimeSource, tempPrime, buttonDrawSize / 2, 45, 255);
    cmdBuildPrimeSource.display();
    cmdBuildPrimeIcon.setTexture(cmdBuildPrimeSource.getTexture());

    if (!cmdBuildFighterSource.create(buttonDrawSize.x, buttonDrawSize.y))
    {
        throw runtime_error("Can't create draw texture.");
    }
    drawFighter(&cmdBuildFighterSource, buttonDrawSize / 2, 45, NEUTRAL_TEAM_COLOR, 255);
    cmdBuildFighterSource.display();
    cmdBuildFighterIcon.setTexture(cmdBuildFighterSource.getTexture());

    if (!cmdBuildGatewaySource.create(buttonDrawSize.x, buttonDrawSize.y))
    {
        throw runtime_error("Can't create draw texture.");
    }
    boost::shared_ptr<Gateway> tempGateway(new Gateway(-1, vector2fp::zero));
    drawGateway(&cmdBuildGatewaySource, tempGateway, buttonDrawSize / 2, 45, NEUTRAL_TEAM_COLOR, 255);
    cmdBuildGatewaySource.display();
    cmdBuildGatewayIcon.setTexture(cmdBuildGatewaySource.getTexture());

    if (!cmdBuildTurretSource.create(buttonDrawSize.x, buttonDrawSize.y))
    {
        throw runtime_error("Can't create draw texture.");
    }
    drawTurret(&cmdBuildTurretSource, buttonDrawSize / 2, 45, NEUTRAL_TEAM_COLOR, 255);
    cmdBuildTurretSource.display();
    cmdBuildTurretIcon.setTexture(cmdBuildTurretSource.getTexture());
}

sf::Sprite* getSpriteForKeyButtonMsg(KeyButtonMsg keyButtonMsg)
{
    switch (keyButtonMsg)
    {
        case WarpIn:
        {
            return &cmdWarpInIcon;
            break;
        }
        case WarpOut:
        {
            return &cmdWarpOutIcon;
            break;
        }
        case Stop:
        {
            return &cmdStopIcon;
            break;
        }
        case Invest:
        {
            return &cmdInvestIcon;
            break;
        }
        case Fetch:
        {
            return &cmdCollectIcon;
            break;
        }
        case Attack:
        {
            return &cmdAttackIcon;
            break;
        }
        case AttackScuttle:
        {
            return &cmdScuttleIcon;
            break;
        }
        case BuildPrime:
        {
            return &cmdBuildPrimeIcon;
            break;
        }
        case BuildFighter:
        {
            return &cmdBuildFighterIcon;
            break;
        }
        case BuildGateway:
        {
            return &cmdBuildGatewayIcon;
            break;
        }
        case BuildTurret:
        {
            return &cmdBuildTurretIcon;
            break;
        }
    }
    throw runtime_error("Can't find sprite for that KeyButtonMsg");
}

sf::View getUXView()
{
    return uxView;
}
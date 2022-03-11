#include <boost/shared_ptr.hpp>
#include <iostream>
#include <fstream>
#include "engine.h"
#include "graphics.h"
#include "common.h"
#include "config.h"

using namespace std;

sf::Font mainFont;

sf::RenderWindow setupGraphics()
{
    if (!mainFont.loadFromFile("/usr/share/fonts/truetype/msttcorefonts/Andale_Mono.ttf"))
        throw runtime_error("Can't load font");

    return sf::RenderWindow(sf::VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), "Coinfight Client", sf::Style::Close | sf::Style::Titlebar);
}

sf::Color playerAddressToColor(string address)
{
    int vals[3];
    for (uint i=0; i<3; i++)
    {
        string charStr = address.substr(2 + i, 1);
        unsigned int intVal = std::stoul(charStr, nullptr, 16);
        vals[i] = (intVal / 15.0) * 255;
    }
    return sf::Color(vals[0], vals[1], vals[2]);
}

void drawEntity(sf::RenderWindow *window, boost::shared_ptr<Entity> entity)
{
    if (boost::shared_ptr<Prime> castedEntity = boost::dynamic_pointer_cast<Prime, Entity>(entity))
    {
        sf::CircleShape circle(3);
        circle.setOrigin(circle.getRadius(), circle.getRadius());
        circle.setFillColor(sf::Color::Blue);
        circle.setPosition(castedEntity->pos.x, castedEntity->pos.y);
        window->draw(circle);
    }
    else if (boost::shared_ptr<Gateway> castedEntity = boost::dynamic_pointer_cast<Gateway, Entity>(entity))
    {
        sf::CircleShape circle(5);
        circle.setOrigin(circle.getRadius(), circle.getRadius());
        circle.setFillColor(sf::Color::Red);
        circle.setPosition(castedEntity->pos.x, castedEntity->pos.y);
        window->draw(circle);
    }
    else if (boost::shared_ptr<GoldPile> castedEntity = boost::dynamic_pointer_cast<GoldPile, Entity>(entity))
    {
        sf::CircleShape circle(5);
        circle.setOrigin(circle.getRadius(), circle.getRadius());
        circle.setOutlineColor(sf::Color(100,100,100));
        circle.setOutlineThickness(1);
        circle.setFillColor(castedEntity->gold.getInt() > 0 ? sf::Color::Yellow : sf::Color::Transparent);
        circle.setPosition(castedEntity->pos.x, castedEntity->pos.y);
        window->draw(circle);
    }
    else
    {
        throw runtime_error("Can't cast that to any known entity!");
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

void display(sf::RenderWindow *window, Game *game, UI ui, int playerIdOrNegativeOne)
{
    window->clear();

    for (unsigned int i = 0; i < game->entities.size(); i++)
    {
        if (game->entities[i])
            drawEntity(window, game->entities[i]);
    }

    vector<sf::String> outputStrings;

    if (playerIdOrNegativeOne >= 0)
    {
        uint playerId = playerIdOrNegativeOne;
        outputStrings.push_back(game->players[playerId].credit.getDollarString());

        drawOutputStrings(window, outputStrings);
    }
    
    window->display();
}
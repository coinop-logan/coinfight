#include <SFML/Graphics.hpp>
#include <GL/gl.h>
#include <GL/glu.h>
#include <boost/shared_ptr.hpp>
#include "engine.h"

sf::Font mainFont;

sf::RenderWindow setupGraphics()
{
    if (!mainFont.loadFromFile("/usr/share/fonts/truetype/msttcorefonts/Andale_Mono.ttf"))
        throw runtime_error("Can't load font");

    return sf::RenderWindow(sf::VideoMode(640, 480), "OpenGL Test", sf::Style::Close | sf::Style::Titlebar);
}

void drawEntity(sf::RenderWindow &window, boost::shared_ptr<Entity> entity)
{
    if (boost::shared_ptr<Prime> castedEntity = boost::dynamic_pointer_cast<Prime, Entity>(entity))
    {
        sf::CircleShape circle(7);
        circle.setOrigin(circle.getRadius(), circle.getRadius());
        circle.setFillColor(sf::Color::Blue);
        circle.setPosition(castedEntity->pos.x, castedEntity->pos.y);
        window.draw(circle);
    }
    else if (boost::shared_ptr<Gateway> castedEntity = boost::dynamic_pointer_cast<Gateway, Entity>(entity))
    {
        sf::CircleShape circle(10);
        circle.setOrigin(circle.getRadius(), circle.getRadius());
        circle.setFillColor(sf::Color::Red);
        circle.setPosition(castedEntity->pos.x, castedEntity->pos.y);
        window.draw(circle);
    }
    else if (boost::shared_ptr<GoldPile> goldPile = boost::dynamic_pointer_cast<GoldPile, Entity>(entity))
    {
        int size = ceil(sqrt(goldPile->gold.getInt() / 70.0));
        if (size > 0)
        {
            sf::CircleShape circle(size);
            circle.setOrigin(circle.getRadius(), circle.getRadius());
            circle.setFillColor(sf::Color::Yellow);
            circle.setPosition(goldPile->pos.x, goldPile->pos.y);
            window.draw(circle);
        }
    }
    else
    {
        throw runtime_error("Can't cast that to any known entity!");
    }
}

void drawOutputStrings(sf::RenderWindow &window, vector<sf::String> strings)
{
    for (uint i=0; i<strings.size(); i++)
    {
        sf::Text text(strings[i], mainFont, 16);
        text.setFillColor(sf::Color(150, 150, 150));

        float width = text.getLocalBounds().width;
        int x, y;
        x = window.getSize().x - width - 10;
        y = 6 + i * 20;
        text.setPosition(sf::Vector2f(x, y));

        window.draw(text);
    }
}

void display(const Game &game, sf::RenderWindow &window)
{
    window.clear();

    for (unsigned int i = 0; i < game.entities.size(); i++)
    {
        if (game.entities[i])
            drawEntity(window, game.entities[i]);
    }

    vector<sf::String> outputStrings;
    
    Coins playerCredit = game.playerCredit;
    outputStrings.push_back(playerCredit.getDollarString());

    drawOutputStrings(window, outputStrings);

    window.display();
}


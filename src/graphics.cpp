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

void drawEntity(const Game &game, sf::RenderWindow &window, boost::shared_ptr<Entity> entity)
{
    if (boost::shared_ptr<Prime> prime = boost::dynamic_pointer_cast<Prime, Entity>(entity))
    {
        if (prime->drawLineToTarget)
        {
            Target target = prime->getTarget();
            if (optional<vector2f> targetPos = target.getPoint(game))
            {
                sf::Vertex line[] =
                {
                    sf::Vertex(sf::Vector2f(prime->pos.x, prime->pos.y)),
                    sf::Vertex(sf::Vector2f(targetPos->x, targetPos->y))
                };

                window.draw(line, 2, sf::Lines);
            }
        }

        sf::CircleShape circle(7);
        circle.setOrigin(circle.getRadius(), circle.getRadius());
        circle.setFillColor(sf::Color::Blue);
        circle.setPosition(prime->pos.x, prime->pos.y);
        window.draw(circle);

        int heldGold = prime->heldGold.getInt();
        if (heldGold > 0)
        {
            int goldBarWidth = heldGold / 30;
            sf::RectangleShape goldBar(sf::Vector2f(goldBarWidth, 2));
            goldBar.setOrigin(sf::Vector2f(goldBarWidth/2, 1));
            goldBar.setOutlineColor(sf::Color(100, 100, 100));
            goldBar.setOutlineThickness(1);
            goldBar.setFillColor(sf::Color::Yellow);
            goldBar.setPosition(prime->pos.x, prime->pos.y + 12);
            window.draw(goldBar);
        }
    }
    else if (boost::shared_ptr<Gateway> gateway = boost::dynamic_pointer_cast<Gateway, Entity>(entity))
    {
        sf::RectangleShape rectangle(sf::Vector2f(16,16));
        rectangle.setOrigin(8, 8);
        rectangle.setFillColor(sf::Color::Red);
        rectangle.setPosition(gateway->pos.x, gateway->pos.y);
        window.draw(rectangle);
    }
    else if (boost::shared_ptr<GoldPile> goldPile = boost::dynamic_pointer_cast<GoldPile, Entity>(entity))
    {
        int size = ceil(sqrt(goldPile->gold.getInt() / 70.0)) + 1;
        if (size > 1)
        {
            sf::CircleShape triangle(size, 3);
            triangle.setOrigin(triangle.getRadius(), triangle.getRadius());
            triangle.setFillColor(sf::Color::Yellow);
            triangle.setPosition(goldPile->pos.x, goldPile->pos.y);
            window.draw(triangle);
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
            drawEntity(game, window, game.entities[i]);
    }

    vector<sf::String> outputStrings;
    
    Coins playerCredit = game.playerCredit;
    outputStrings.push_back(playerCredit.getDollarString());

    drawOutputStrings(window, outputStrings);

    window.display();
}


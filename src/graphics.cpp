#include <SFML/Graphics.hpp>
#include <GL/gl.h>
#include <GL/glu.h>
#include <boost/shared_ptr.hpp>
#include "engine.h"

sf::Font mainFont;
sf::Text smallGrayText;

sf::RenderWindow setupGraphics()
{
    if (!mainFont.loadFromFile("/usr/share/fonts/truetype/msttcorefonts/Andale_Mono.ttf"))
        throw runtime_error("Can't load font");

    smallGrayText.setFont(mainFont);
    smallGrayText.setCharacterSize(12);
    smallGrayText.setFillColor(sf::Color(150, 150, 150));

    smallGrayText.setString("Hello world");

    return sf::RenderWindow(sf::VideoMode(640, 480), "OpenGL Test", sf::Style::Close | sf::Style::Titlebar);
}

void drawEntity(sf::RenderWindow &window, boost::shared_ptr<Entity> entity)
{
    if (boost::shared_ptr<Prime> castedEntity = boost::dynamic_pointer_cast<Prime, Entity>(entity))
    {
        sf::CircleShape circle(3);
        circle.setOrigin(circle.getRadius(), circle.getRadius());
        circle.setFillColor(sf::Color::Blue);
        circle.setPosition(castedEntity->pos.x, castedEntity->pos.y);
        window.draw(circle);
    }
    else if (boost::shared_ptr<Gateway> castedEntity = boost::dynamic_pointer_cast<Gateway, Entity>(entity))
    {
        sf::CircleShape circle(5);
        circle.setOrigin(circle.getRadius(), circle.getRadius());
        circle.setFillColor(sf::Color::Red);
        circle.setPosition(castedEntity->pos.x, castedEntity->pos.y);
        window.draw(circle);
    }
    else if (boost::shared_ptr<GoldPile> castedEntity = boost::dynamic_pointer_cast<GoldPile, Entity>(entity))
    {
        sf::CircleShape circle(5);
        circle.setOrigin(circle.getRadius(), circle.getRadius());
        circle.setOutlineColor(sf::Color(100,100,100));
        circle.setOutlineThickness(1);
        circle.setFillColor(castedEntity->gold.getInt() > 0 ? sf::Color::Yellow : sf::Color::Transparent);
        circle.setPosition(castedEntity->pos.x, castedEntity->pos.y);
        window.draw(circle);
    }
    else
    {
        throw runtime_error("Can't cast that to any known entity!");
    }
    window.draw(smallGrayText);
    smallGrayText.setString("hiiiiii");
    window.draw(smallGrayText);
}
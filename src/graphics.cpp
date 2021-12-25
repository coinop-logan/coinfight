#include <SFML/Graphics.hpp>
#include <GL/gl.h>
#include <GL/glu.h>
#include <boost/shared_ptr.hpp>
#include "engine.h"

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
        circle.setFillColor(sf::Color::Yellow);
        circle.setPosition(castedEntity->pos.x, castedEntity->pos.y);
        window.draw(circle);
    }
    else
    {
        throw runtime_error("Can't cast that to any known entity!");
    }
}
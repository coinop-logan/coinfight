// #include <GL/glew.h>
#include <GL/gl.h>
// #include <glm/glm.hpp>

#include <SFML/Graphics.hpp>
#include <SFML/OpenGL.hpp>
#include <GL/glu.h>
#include <iostream>
#include <boost/shared_ptr.hpp>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <vector>
#include <string>

using namespace std;
using namespace boost::asio::ip;

sf::RenderWindow setupGraphics()
{
    return sf::RenderWindow(sf::VideoMode(640, 480), "OpenGL Test", sf::Style::Close | sf::Style::Titlebar);
}

int main()
{
    sf::RenderWindow window = setupGraphics();
    sf::Event event;

    while (window.isOpen())
    {
        while (window.pollEvent(event))
        {
            switch (event.type)
            {
            case sf::Event::Closed:
                window.close();
                break;
            default:
                break;
            }
        }
    }

    return 0;
}
#include <iostream>
#include <SFML/Graphics.hpp>
#include "interface/graphics/common.h"
#include "common/myvectors.h"

using namespace std;

const vector2i LAUNCHER_SCREEN_SIZE(800, 500);

sf::RenderWindow* setupGraphics()
{
    sf::VideoMode videoMode(LAUNCHER_SCREEN_SIZE.x, LAUNCHER_SCREEN_SIZE.y);
    int flags = sf::Style::Close | sf::Style::Titlebar;

    sf::RenderWindow* window = new sf::RenderWindow(videoMode, "Coinfight Launcher", flags);

    loadFonts();

    return window;
}

void cleanupGraphics(sf::RenderWindow* window)
{
    delete window;
}

void display(sf::RenderWindow* window)
{
    window->clear(sf::Color(0, 0, 100));

    displayTitle(window, 20);

    window->display();
}

int main(int argc, char *argv[])
{
    sf::RenderWindow* window = setupGraphics();

    while (window->isOpen())
    {
        sf::Event event;
        while (window->pollEvent(event))
        {
            switch (event.type)
            {
                case sf::Event::Closed:
                {
                    window->close();
                    break;
                }
            }
        }
    }

    cleanupGraphics(window);
}
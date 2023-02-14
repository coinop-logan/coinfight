#include <iostream>
#include <SFML/Graphics.hpp>
#include <chrono>
#include "interface/graphics/common.h"
#include "interface/common.h"
#include "common/myvectors.h"
#include "config.h"

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

void display(sf::RenderWindow* window, sf::Font* humanFont)
{
    window->clear(sf::Color(0, 0, 50));

    displayTitle(window, 20);

    sf::Text launcherText("launcher", *humanFont, 36);
    launcherText.setOrigin(sf::Vector2f(launcherText.getLocalBounds().width / 2, 0));
    launcherText.setPosition(sf::Vector2f(getScreenSize(window).x / 2, 130));
    launcherText.setFillColor(sf::Color(255, 255, 255, 150));
    launcherText.setStyle(sf::Text::Style::Italic);
    window->draw(launcherText);

    window->display();
}

int main(int argc, char *argv[])
{
    sf::RenderWindow* window = setupGraphics();
    sf::Font* humanFont = getHumanFont();
    sf::Font* fwFont = getFWFont();

    chrono::time_point<chrono::system_clock, chrono::duration<double>> nextFrameStart(chrono::system_clock::now());

    while (window->isOpen())
    {
        chrono::time_point<chrono::system_clock, chrono::duration<double>> now(chrono::system_clock::now());
        if (now < nextFrameStart)
            continue;
        
        nextFrameStart += ONE_FRAME;

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

        display(window, humanFont);
    }

    cleanupGraphics(window);
}
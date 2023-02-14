#include <iostream>
#include <SFML/Graphics.hpp>

using namespace std;

sf::RenderWindow* setupGraphics()
{
    sf::VideoMode videoMode(800, 500);
    int flags = sf::Style::Close | sf::Style::Titlebar;

    sf::RenderWindow* window = new sf::RenderWindow(videoMode, "Coinfight Launcher", flags);

    return window;
}

void cleanupGraphics(sf::RenderWindow* window)
{
    delete window;
}

int main(int argc, char *argv[])
{
    sf::RenderWindow* window = setupGraphics();

    cout << "hi there" << endl;

    cleanupGraphics(window);
}
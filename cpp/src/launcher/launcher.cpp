#include <iostream>
#include <SFML/Graphics.hpp>
#include <chrono>
#include <curl/curl.h>
#include <sstream>
#include "interface/graphics/common.h"
#include "interface/common.h"
#include "common/myvectors.h"
#include "config.h"

using namespace std;

const vector2i LAUNCHER_SCREEN_SIZE(800, 500);

sf::RenderWindow* setupGraphics();
void cleanupGraphics(sf::RenderWindow* window);
void display(sf::RenderWindow* window, sf::Font* humanFont);

optional<vector<int>> checkLocalFilesVersion();
optional<vector<int>> checkServerVersion();

string versionToString(vector<int>);

// enum AppState
// {
//     CheckingLocalFiles,
//     CheckingRemoteVersion,
//     ReadyForUpdate,
//     Updating,
//     ReadyToPlay
// };

int main(int argc, char *argv[])
{
    curl_global_init(0);

    sf::RenderWindow* window = setupGraphics();
    sf::Font* humanFont = getHumanFont();
    sf::Font* fwFont = getFWFont();

    // check local files
    // optional<vector<int>> localFilesVersion = checkLocalFilesVersion();

    // check remote version
    optional<vector<int>> serverVersion = checkServerVersion();
    if (serverVersion)
    {
        cout << "version: " << versionToString(*serverVersion) << endl;
    }
    else
    {
        cout << "no version found!" << endl;
    }

    return 0;

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


optional<vector<int>> checkLocalFilesVersion()
{
    return {};
}

optional<vector<int>> checkServerVersion()
{
    CURL* c = curl_easy_init();

    curl_easy_cleanup(c);

    return {};
}

string versionToString(vector<int> version)
{
    stringstream ss;
    for (unsigned int i=0; i<version.size(); i++)
    {
        if (i != 0)
        {
            ss << ".";
        }
        ss << version[i];
    }

    return ss.str();
}
#include <SFML/Graphics.hpp>
#include <string>
#include <boost/shared_ptr.hpp>
#include "myvectors.h"
#include "interface.h"

#ifndef MENU_H
#define MENU_H

using namespace std;

const int MAIN_MENU_BUTTON_TEXT_OFFSET_Y = -1;
const int MAIN_MENU_BUTTON_FONT_SIZE = 18;
const int MAIN_MENU_BUTTON_HEIGHT = 22;
const int MAIN_MENU_BUTTON_SPACING = 8;
const int MAIN_MENU_PADDING = 5;
const int MAIN_MENU_WIDTH = 200;

class Button
{
public:
    vector2i p1, p2;
    vector2i mousePos;
    bool pressed, mouseover;
    Button(vector2i p1, vector2i p2);
    void registerMouseMove(vector2i);
    void registerPress();
    bool registerRelease();
    void draw(sf::RenderWindow*);
    virtual void drawContent(sf::RenderWindow*) {}
    vector2i getDimensions()
    {
        return (p2 - p1);
    }
};

class TextButton : public Button
{
    string text;
    sf::Font font;
    int fontSize;
    int textOffsetY;
public:
    TextButton(vector2i p1, vector2i, string, sf::Font, int fontSize, int textOffsetY);
    void drawContent(sf::RenderWindow*);
};

enum MainMenuEvent
{
    StartLocal,
    StartLocalDebug
};

template<class T>
struct BoundButton
{
    boost::shared_ptr<Button> button;
    T eventMsg;
};

class MainMenu
{
    vector2i p1, p2;
    vector<BoundButton<MainMenuEvent>> boundButtons;
public:
    MainMenu(vector<tuple<string, MainMenuEvent>> buttonInfos, sf::Font font);
    optional<MainMenuEvent> processEvent(sf::Event);
    void draw(sf::RenderWindow*);
};

#endif // MENU_H
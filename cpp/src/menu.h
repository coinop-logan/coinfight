#include <SFML/Graphics.hpp>
#include <string>
#include "myvectors.h"
#include "interface.h"

#ifndef MENU_H
#define MENU_H

using namespace std;

const int MAIN_MENU_BUTTON_TEXT_OFFSET_Y = 20;
const int MAIN_MENU_BUTTON_FONT_SIZE = 18;
const int MAIN_MENU_BUTTON_HEIGHT = 22;
const int MAIN_MENU_BUTTON_SPACING = 8;
const int MAIN_MENU_PADDING = 5;
const int MAIN_MENU_WIDTH = 200;

class Button
{
public:
    vector2i p1, p2;
    void (*onClick)();
    vector2i mousePos;
    bool pressed, mouseover;
    Button(vector2i p1, vector2i p2, void (*onClick)());
    void registerMouseMove(vector2i);
    void registerClick();
    void registerRelease();
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
    TextButton(vector2i p1, vector2i, string, sf::Font, int fontSize, int textOffsetY, void (*onClick)());
    void drawContent(sf::RenderWindow*);
};

class MainMenu
{
    vector2i p1, p2;
    vector<TextButton> buttons;
public:
    MainMenu(vector<tuple<string, void (*)()>> buttonInfos, sf::Font font);
    void processEvent(sf::Event);
    void draw(sf::RenderWindow*);
};

#endif // MENU_H
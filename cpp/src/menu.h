#include <SFML/Graphics.hpp>
#include <string>
#include "myvectors.h"

#ifndef MENU_H
#define MENU_H

using namespace std;

const vector2i BUTTON_TEXT_PADDING = vector2i(5, 5);

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
public:
    TextButton(vector2i p1, void (*onClick)(), string, sf::Font, int fontSize);
    void drawContent(sf::RenderWindow*);
};

class Menu
{

};

#endif // MENU_H
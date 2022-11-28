#include <iostream>
#include "menu.h"
#include "common.h"
#include "myvectors.h"

using namespace std;

bool collides(vector2i p1, vector2i p2, vector2i point)
{
    return (
        point.x >= p1.x &&
        point.x <= p2.x &&
        point.y >= p1.y &&
        point.y <= p2.y
    );
}

Button::Button(vector2i p1, vector2i p2, void (*onClick)())
    : p1(p1), p2(p2), onClick(onClick), mousePos(), pressed(false), mouseover(false)
    {}
void Button::registerMouseMove(vector2i newMousePos)
{
    mouseover = collides(p1, p2, newMousePos);
    mousePos = newMousePos;
}
void Button::registerClick()
{
    if (mouseover)
    {
        pressed = true;
    }
}
void Button::registerRelease()
{
    if (pressed && mouseover)
    {
        onClick();
    }
    pressed = false;
}
void Button::draw(sf::RenderWindow* window)
{
    sf::RectangleShape buttonRect(toSFVec(vector2fl(p2 - p1)));
    buttonRect.setPosition(toSFVec(vector2fl(p1)));

    buttonRect.setOutlineThickness(1);
    if (!mouseover)
    {
        buttonRect.setFillColor(sf::Color::Black);
        buttonRect.setOutlineColor(sf::Color(150, 150, 255, 100));
    }
    else if (!pressed) // mouseover but not pressed
    {
        buttonRect.setFillColor(sf::Color::Black);
        buttonRect.setOutlineColor(sf::Color(150, 150, 255));
    }
    else // mouseover && pressed
    {
        buttonRect.setFillColor(sf::Color(50, 50, 100));
        buttonRect.setOutlineColor(sf::Color(150, 150, 255));
    }

    window->draw(buttonRect);
    drawContent(window);
}


vector2i calcTextButtonP2(vector2i p1, string s, sf::Font font, int fontSize)
{
    sf::Text renderedText(sf::String(s), font, fontSize);
    auto boundingRect = renderedText.getLocalBounds();
    return p1 + vector2i(boundingRect.width, fontSize) + (BUTTON_TEXT_PADDING * 2);
}

sf::Text rtext2;

TextButton::TextButton(vector2i p1, void (*onClick)(), string text, sf::Font font, int fontSize)
    : Button(p1, calcTextButtonP2(p1, text, font, fontSize), onClick), text(text), font(font), fontSize(fontSize)
{
}
extern sf::Font mainFont;
void TextButton::drawContent(sf::RenderWindow* window)
{
    cout << text << endl;
    sf::Text renderedText(sf::String(text), font, fontSize);

    renderedText.setPosition(toSFVec(vector2fl(p1 + BUTTON_TEXT_PADDING - vector2i(0, 3))));
    renderedText.setFillColor(sf::Color::White);

    window->draw(renderedText);
}
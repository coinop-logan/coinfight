#include <SFML/Graphics.hpp>
#include <string>
#include <boost/shared_ptr.hpp>
#include "myvectors.h"
#include "common.h"
#include "interface.h"

#ifndef MENU_H
#define MENU_H

using namespace std;

const int MAIN_MENU_BUTTON_TEXT_OFFSET_Y = -1;
const int MAIN_MENU_BUTTON_FONT_SIZE = 18;
const int MAIN_MENU_BUTTON_HEIGHT = 22;
const int MAIN_MENU_BUTTON_SPACING = 8;
const int MAIN_MENU_PADDING = 5;
const int MAIN_MENU_WIDTH = 400;

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

template<class EventMsg>
struct BoundButton
{
    boost::shared_ptr<Button> button;
    EventMsg eventMsg;
};

template<class EventMsg>
class MainMenu
{
    vector2i p1, p2;
    vector<BoundButton<EventMsg>> boundButtons;
public:
    MainMenu(vector<tuple<string, EventMsg>> buttonInfos, sf::Font font)
    {
        vector2i buttonDimensions =
            vector2i(
                MAIN_MENU_WIDTH - (MAIN_MENU_PADDING * 2),
                MAIN_MENU_BUTTON_HEIGHT
            );
        
        vector2i centerPos = screenDimensions / 2;

        int height =
            (buttonInfos.size() * MAIN_MENU_BUTTON_HEIGHT) // total button height
        + (buttonInfos.size() == 0 ? 0 : (buttonInfos.size() - 1) * MAIN_MENU_BUTTON_SPACING)
        + (MAIN_MENU_PADDING * 2);
        
        int width = MAIN_MENU_WIDTH;

        vector2i dimensions(width, height);
        vector2i halfDimensions = dimensions / 2;
        
        p1 = centerPos - halfDimensions;
        p2 = centerPos + halfDimensions;

        for (uint i=0; i<buttonInfos.size(); i++)
        {
            vector2i buttonP1 =
                p1
            + vector2i(MAIN_MENU_PADDING, MAIN_MENU_PADDING)
            + vector2i
                    ( 0,
                    (i * (MAIN_MENU_BUTTON_HEIGHT + MAIN_MENU_BUTTON_SPACING))
                    );

            boost::shared_ptr<Button> newButton(new TextButton
            ( buttonP1,
                buttonDimensions,
                get<0>(buttonInfos[i]),
                font,
                MAIN_MENU_BUTTON_FONT_SIZE,
                MAIN_MENU_BUTTON_TEXT_OFFSET_Y
            )
            );

            boundButtons.push_back(
                {newButton, get<1>(buttonInfos[i])}
            );
        }
    }
    optional<EventMsg> processEvent(sf::Event event)
    {
        optional<EventMsg> menuEventMsg;

        switch (event.type)
        {
            case sf::Event::MouseMoved:
            {
                for (uint i=0; i<boundButtons.size(); i++)
                {
                    boundButtons[i].button->registerMouseMove(vector2i(event.mouseMove.x, event.mouseMove.y));
                }
                break;
            }
            case sf::Event::MouseButtonPressed:
            {
                for (uint i=0; i<boundButtons.size(); i++)
                {
                    boundButtons[i].button->registerPress();
                }
                break;
            }
            case sf::Event::MouseButtonReleased:
            {
                for (uint i=0; i<boundButtons.size(); i++)
                {
                    if (boundButtons[i].button->registerRelease())
                    {
                        menuEventMsg = boundButtons[i].eventMsg;
                    }
                }
                break;
            }
            default:
            {}
        }
        
        return menuEventMsg;
    }
    void draw(sf::RenderWindow* window)
    {
        sf::RectangleShape mainBox(toSFVec(vector2fl(p2 - p1)));
        mainBox.setPosition(toSFVec(vector2fl(p1)));
        mainBox.setFillColor(sf::Color::Black);
        mainBox.setOutlineThickness(1);
        mainBox.setOutlineColor(sf::Color(0, 0, 255));
        window->draw(mainBox);

        for (uint i=0; i<boundButtons.size(); i++)
        {
            boundButtons[i].button->draw(window);
        }
    }
};

#endif // MENU_H
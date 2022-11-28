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

Button::Button(vector2i p1, vector2i p2)
    : p1(p1), p2(p2), mousePos(), pressed(false), mouseover(false)
    {}
void Button::registerMouseMove(vector2i newMousePos)
{
    mouseover = collides(p1, p2, newMousePos);
    mousePos = newMousePos;
}
void Button::registerPress()
{
    if (mouseover)
    {
        pressed = true;
    }
}
bool Button::registerRelease()
{
    if (pressed && mouseover)
    {
        pressed = false;
        return true;
    }
    else
    {
        pressed = false;
        return false;
    }
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


// vector2i calcTextButtonP2(vector2i p1, string s, sf::Font font, int fontSize)
// {
//     sf::Text renderedText(sf::String(s), font, fontSize);
//     auto boundingRect = renderedText.getLocalBounds();
//     return p1 + vector2i(boundingRect.width, fontSize) + (BUTTON_TEXT_PADDING * 2);
// }

TextButton::TextButton(vector2i p1, vector2i dimensions, string text, sf::Font font, int fontSize, int textOffsetY)
    : Button(p1, (p1 + dimensions)), text(text), font(font), fontSize(fontSize), textOffsetY(textOffsetY)
{
}

void TextButton::drawContent(sf::RenderWindow* window)
{
    sf::Text renderedText(sf::String(text), font, fontSize);
    
    // center text
    int spaceForText = (p2.x - p1.x);

    auto boundingRect = renderedText.getLocalBounds();
    int textWidth = boundingRect.width;

    int leftoverSpace = spaceForText - textWidth;
    int offsetX = leftoverSpace / 2;

    renderedText.setPosition(toSFVec(vector2fl(p1 + vector2i(offsetX, textOffsetY))));
    renderedText.setFillColor(sf::Color::White);

    window->draw(renderedText);
}


MainMenu::MainMenu(vector<tuple<string, MainMenuEvent>> buttonInfos, sf::Font font)
{
    vector2i buttonDimensions =
        vector2i(
            MAIN_MENU_WIDTH - (MAIN_MENU_PADDING * 2),
            MAIN_MENU_BUTTON_HEIGHT
        );
    
    vector2i centerPos = screenDimensions / 2;

    int height =
        (buttonInfos.size() * (MAIN_MENU_BUTTON_HEIGHT + MAIN_MENU_BUTTON_SPACING))
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

void MainMenu::draw(sf::RenderWindow* window)
{
    sf::RectangleShape mainBox(toSFVec(vector2fl(p2 - p1)));
    mainBox.setPosition(toSFVec(vector2fl(p1)));
    mainBox.setOutlineColor(sf::Color(0, 0, 255));
    mainBox.setOutlineThickness(1);
    window->draw(mainBox);

    for (uint i=0; i<boundButtons.size(); i++)
    {
        boundButtons[i].button->draw(window);
    }
}

optional<MainMenuEvent> MainMenu::processEvent(sf::Event event)
{
    optional<MainMenuEvent> menuEventMsg;

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
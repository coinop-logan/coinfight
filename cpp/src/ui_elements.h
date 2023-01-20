#include <SFML/Graphics.hpp>
#include <string>
#include <boost/shared_ptr.hpp>
#include "myvectors.h"
#include "common.h"

#ifndef UI_ELEMENTS_H
#define UI_ELEMENTS_H

using namespace std;

const int WINDOW_PADDING = 5;

const int MAIN_MENU_BUTTON_TEXT_OFFSET_Y = -1;
const int MAIN_MENU_BUTTON_FONT_SIZE = 18;
const int MAIN_MENU_BUTTON_HEIGHT = 22;
const int MAIN_MENU_BUTTON_SPACING = 8;
const int MAIN_MENU_WIDTH = 400;

const int NOTICE_WINDOW_FONT_SIZE = 18;
const int NOTICE_WINDOW_WIDTH = 600;

const vector2i LOGIN_WINDOW_DIMENSIONS(600, 500);
const vector2i GIFT_WINDOW_DIMENSIONS(600, 300);

const vector2i COPYPASTE_BUTTON_DIMENSIONS(50, 50);

void loadIcons();

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
    sf::Font* font;
    int fontSize;
    int textOffsetY;
public:
    TextButton(vector2i p1, vector2i dimensions, string, sf::Font*, int fontSize, int textOffsetY);
    void drawContent(sf::RenderWindow*);
};

class ImageButton : public Button
{
public:
    sf::Sprite sprite;
    ImageButton(vector2i p1, vector2i dimensions, sf::Sprite sprite);
    void changeImage(sf::Sprite sprite);
    void drawContent(sf::RenderWindow*);
};

template<class EventMsg>
struct BoundButton
{
    boost::shared_ptr<Button> button;
    EventMsg eventMsg;
};

class Window
{
public:
    vector2i p1, p2;
    Window() {}
    Window(vector2i p1, vector2i p2);
    virtual void drawContent(sf::RenderWindow* window, vector2i drawOffset) {}
    void draw(sf::RenderWindow* window);
};

template<class EventMsg>
class MainMenu : public Window
{
    vector<BoundButton<EventMsg>> boundButtons;
public:
    MainMenu(sf::RenderWindow* window, vector<tuple<string, EventMsg>> buttonInfos, sf::Font* font)
    {
        // Window::p1 and p2 will be set a bit later

        vector2i buttonDimensions =
            vector2i(
                MAIN_MENU_WIDTH - (WINDOW_PADDING * 2),
                MAIN_MENU_BUTTON_HEIGHT
            );

        vector2i centerPos = getScreenSize(window) / 2;

        int height =
            (buttonInfos.size() * MAIN_MENU_BUTTON_HEIGHT) // total button height
          + (buttonInfos.size() == 0 ? 0 : (buttonInfos.size() - 1) * MAIN_MENU_BUTTON_SPACING)
          + (WINDOW_PADDING * 2);

        int width = MAIN_MENU_WIDTH;

        vector2i dimensions(width, height);
        vector2i halfDimensions = dimensions / 2;

        p1 = centerPos - halfDimensions;
        p2 = centerPos + halfDimensions;

        for (unsigned int i=0; i<buttonInfos.size(); i++)
        {
            vector2i buttonP1 =
                p1
              + vector2i(WINDOW_PADDING, WINDOW_PADDING)
              + vector2i( 0, (i * (MAIN_MENU_BUTTON_HEIGHT + MAIN_MENU_BUTTON_SPACING)));

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
                for (unsigned int i=0; i<boundButtons.size(); i++)
                {
                    boundButtons[i].button->registerMouseMove(vector2i(event.mouseMove.x, event.mouseMove.y));
                }
                break;
            }
            case sf::Event::MouseButtonPressed:
            {
                for (unsigned int i=0; i<boundButtons.size(); i++)
                {
                    boundButtons[i].button->registerPress();
                }
                break;
            }
            case sf::Event::MouseButtonReleased:
            {
                for (unsigned int i=0; i<boundButtons.size(); i++)
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
    void drawContent(sf::RenderWindow* window, vector2i drawOffset)
    {
        for (unsigned int i=0; i<boundButtons.size(); i++)
        {
            boundButtons[i].button->draw(window);
        }
    }
};

class NoticeWindow : public Window
{
public:
    string message;
    boost::shared_ptr<TextButton> okayButton;
    sf::Font* font;
    NoticeWindow(vector2i center, string message, sf::Font*);
    sf::Text renderText();
    void drawContent(sf::RenderWindow* window, vector2i drawOffset);
    bool processEvent(sf::Event event);
};

class LoginWindow : public Window
{
public:
    enum Msg
    {
        Back,
        ResponseEntered
    };
    string sigChallenge;
    optional<tuple<Address, string>> addressAndSigResponse;
    boost::shared_ptr<TextButton> backButton;
    boost::shared_ptr<ImageButton> copyButton, pasteButton; // will be set in drawContent. Hacky! Sorry!
    sf::Font* font;
    bool copySuccessful;
    optional<string> errorString;
    LoginWindow(vector2i center, string sigChallenge, sf::Font*);
    void drawContent(sf::RenderWindow* window, vector2i drawOffset);
    optional<Msg> processEvent(sf::Event event);
};

class GiftUnitsWindow : public Window
{
public:
    enum Msg
    {
        Back,
        AddressSubmitted
    };
    optional<string> pastedText;
    optional<Address> submittedAddress;
    boost::shared_ptr<TextButton> backButton;
    boost::shared_ptr<TextButton> submitButton;
    boost::shared_ptr<ImageButton> pasteButton;
    sf::Font* font;
    optional<string> errorString;
    GiftUnitsWindow(vector2i center, sf::Font*);
    void drawContent(sf::RenderWindow* window, vector2i drawOffset);
    optional<Msg> processEvent(sf::Event event);
};

void runNoticeWindow(sf::RenderWindow* window, string message, sf::Font*);

#endif // UI_ELEMENTS_H

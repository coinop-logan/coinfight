#include <SFML/Graphics.hpp>
#include <string>
#include <boost/shared_ptr.hpp>
#include "myvectors.h"
#include "common.h"
#include "entities.h"

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

const vector2i UX_ELEMENT_SPACING(5, 5);
const vector2i UX_BOX_PADDING(10, 10);
const vector2i UX_BOX_SPACING(10, 10);
const sf::Color UX_BOX_BORDER_COLOR(150, 150, 200);
const sf::Color UX_BOX_BACKGROUND_COLOR(0, 0, 0, 200);

const vector2i KEYBUTTON_ICON_BASE_SIZE(19, 19);
const int KEYBUTTON_ICON_SCALE = 3;
const vector2i KEYBUTTON_PADDING(3, 3);
const vector2i KEYBUTTON_SIZE = (KEYBUTTON_ICON_BASE_SIZE * KEYBUTTON_ICON_SCALE) + (KEYBUTTON_PADDING * 2);
const int KEYBUTTON_SPACING = 8;

const vector2i KEYBUTTONBOX_PADDING(5, 5);
const int KEYBUTTON_ROW2_OFFSETX = 20;
const vector2i KEYBUTTONBOX_SIZE(
    KEYBUTTON_ROW2_OFFSETX + KEYBUTTON_SIZE.x * 4 + KEYBUTTON_SPACING * 3 + KEYBUTTONBOX_PADDING.x * 2,
    KEYBUTTON_SIZE.x * 3 + KEYBUTTON_SPACING * 2 + KEYBUTTONBOX_PADDING.y * 2
);

const vector2i UNITINFOBOX_SIZE(350, 350);

const vector2i UNIT_ART_SIZE(120, 120);

const int UNIT_INFO_STATUS_ELEMENT_SPACING = 14;

void loadMenuIcons();

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

struct KeyButtonHintInfo
{
    string name;
    optional<coinsInt> maybeCost;
    char hotkeyChar;
    string description;
    vector<string> bulletPoints;
    KeyButtonHintInfo(string name, optional<coinsInt> maybeCost, char hotkeyChar, string description, vector<string> bulletPoints)
        : name(name), maybeCost(maybeCost), hotkeyChar(hotkeyChar), description(description), bulletPoints(bulletPoints)
        {}
};

enum KeyButtonMsg
{
    WarpIn,
    WarpOut,
    Stop,
    Invest,
    Fetch,
    Attack,
    AttackScuttle,
    BuildPrime,
    BuildFighter,
    BuildGateway,
    BuildTurret
};

struct KeyButtonActionInfo
{
    KeyButtonActionInfo(sf::Sprite* sprite, KeyButtonHintInfo hintInfo, KeyButtonMsg keyButtonMsg);
    sf::Sprite* sprite;
    KeyButtonHintInfo hintInfo;
    KeyButtonMsg keyButtonMsg;
};

class KeyButton : public Button
{
public:
    sf::Keyboard::Key key;
    sf::Text keyCharText;
    optional<KeyButtonActionInfo> maybeActionInfo;
    bool active;
    sf::Clock visualFlashClock;
    KeyButton(vector2i upperLeft, sf::Keyboard::Key key, sf::Text keyCharText);
    void setKeyButtonActionInfo(optional<KeyButtonActionInfo> _actionInfo);
    void draw(sf::RenderWindow*);
    void drawContent(sf::RenderWindow*);
};

class UXBox
{
public:
    vector2i upperLeft, size;
    UXBox(vector2i upperLeft, vector2i size);
    bool pointCollides(vector2i pos);
    virtual void drawContent(sf::RenderWindow* window, vector2i upperLeft) {}
    void draw(sf::RenderWindow* window);
};

class KeyButtonUXBox : public UXBox
{
    vector<KeyButton> keyButtons;
    KeyButton* getKeyButton(sf::Keyboard::Key key);
    sf::Sprite* (*getSpriteForMsg)(KeyButtonMsg);
public:
    KeyButtonUXBox(vector2i upperLeft, sf::Font* font, sf::Sprite* (*getSpriteForMsg)(KeyButtonMsg));
    void setUnitCmdOrThrow(sf::Keyboard::Key key, KeyButtonHintInfo hintInfo, KeyButtonMsg keyButtonMsg);
    void clearActionInfos();
    void returnToDefaultState();
    optional<KeyButtonHintInfo> getMouseoverHintInfo();
    bool registerMouseMove(vector2i);
    bool registerPress(vector2i);
    tuple<bool, optional<tuple<KeyButton*, KeyButtonMsg>>> registerRelease(vector2i);
    optional<tuple<KeyButton*, KeyButtonMsg>> handleKey(sf::Keyboard::Key);
    void drawContent(sf::RenderWindow* window, vector2i upperLeft);
};

class UnitInfoUXBox : public UXBox
{
    sf::Font* font;
    vector<boost::shared_ptr<Unit>>* selectedUnits;
public:
    UnitInfoUXBox(vector2i upperLeft, sf::Font* font, vector<boost::shared_ptr<Unit>>* selectedUnits);
    void drawContent(sf::RenderWindow* window, vector2i upperLeft);
};

#endif // UI_ELEMENTS_H

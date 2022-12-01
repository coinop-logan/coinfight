#include <iostream>
#include "ui_elements.h"
#include "common.h"
#include "myvectors.h"
#include "graphics_helpers.h"

using namespace std;

sf::Texture
    copyActionSource,
    copyDoneSource,
    pasteActionSource,
    pasteDoneSource
;

sf::Sprite
    copyActionIcon,
    copyDoneIcon,
    pasteActionIcon,
    pasteDoneIcon
;

void loadIcons()
{
    if (!copyActionSource.loadFromFile("clipboard-copy-action.png"))
    {
        throw runtime_error("Can't load copy icon");
    }
    copyActionIcon.setTexture(copyActionSource);

    if (!copyDoneSource.loadFromFile("clipboard-copy-done.png"))
    {
        throw runtime_error("Can't load copy done icon");
    }
    copyDoneIcon.setTexture(copyDoneSource);

    if (!pasteActionSource.loadFromFile("clipboard-paste-action.png"))
    {
        throw runtime_error("Can't load paste icon");
    }
    pasteActionIcon.setTexture(pasteActionSource);

    if (!pasteDoneSource.loadFromFile("clipboard-paste-done.png"))
    {
        throw runtime_error("Can't load paste done icon");
    }
    pasteDoneIcon.setTexture(pasteDoneSource);
}

bool collides(vector2i p1, vector2i p2, vector2i point)
{
    return (
        point.x     >= p1.x &&
        point.x - 1 <= p2.x &&
        point.y     >= p1.y &&
        point.y - 1 <= p2.y
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
        buttonRect.setOutlineColor(sf::Color(150, 150, 255, 200));
    }

    window->draw(buttonRect);
    drawContent(window);
}

TextButton::TextButton(vector2i p1, vector2i dimensions, string text, sf::Font* font, int fontSize, int textOffsetY)
    : Button(p1, (p1 + dimensions)), text(text), font(font), fontSize(fontSize), textOffsetY(textOffsetY)
{
}

void TextButton::drawContent(sf::RenderWindow* window)
{
    sf::Text renderedText(sf::String(text), *font, fontSize);
    
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

ImageButton::ImageButton(vector2i p1, vector2i dimensions, sf::Sprite sprite)
    : Button(p1, p1 + dimensions), sprite(sprite)
{}

void ImageButton::drawContent(sf::RenderWindow* window)
{
    auto bounds = sprite.getLocalBounds();
    vector2i dimensions(bounds.width, bounds.height);

    vector2i halfDims = dimensions / 2;
    vector2i buttonCenter = p1 + (getDimensions() / 2);

    vector2i pos = buttonCenter - halfDims;

    sprite.setPosition(toSFVec(vector2fl(pos)));
    window->draw(sprite);
}

void ImageButton::changeImage(sf::Sprite newSprite)
{
    sprite = newSprite;
}

Window::Window(vector2i p1, vector2i p2)
    : p1(p1), p2(p2)
{}

void Window::draw(sf::RenderWindow* window)
{
    sf::RectangleShape mainBox(toSFVec(vector2fl(p2 - p1)));
    mainBox.setPosition(toSFVec(vector2fl(p1)));
    mainBox.setFillColor(sf::Color::Black);
    mainBox.setOutlineThickness(1);
    mainBox.setOutlineColor(sf::Color(0, 0, 255));
    window->draw(mainBox);

    drawContent(window, p1 + vector2i(WINDOW_PADDING, WINDOW_PADDING));
}

LoginWindow::LoginWindow(vector2i center, string sigChallenge, sf::Font* font)
    : sigChallenge(sigChallenge), font(font)
{
    copySuccessful = false;
    vector2i halfDims = LOGIN_WINDOW_DIMENSIONS / 2;
    p1 = center - halfDims;
    p2 = center + halfDims;

    vector2i drawStart = p1 + vector2i(WINDOW_PADDING, WINDOW_PADDING);

    backButton = boost::shared_ptr<TextButton>(
        new TextButton(
            drawStart,
            vector2i(LOGIN_WINDOW_DIMENSIONS.x - WINDOW_PADDING * 2, MAIN_MENU_BUTTON_HEIGHT),
            "Back",
            font,
            MAIN_MENU_BUTTON_FONT_SIZE,
            MAIN_MENU_BUTTON_TEXT_OFFSET_Y
        )
    );
}
void drawStepNum(sf::RenderWindow* window, string numString, sf::Font* font, vector2i drawPos);
uint drawWrappedStepText(sf::RenderWindow* window, string textString, sf::Font* font, vector2i drawPos, uint textBoxWidth);
void LoginWindow::drawContent(sf::RenderWindow* window, vector2i drawOffset)
{
    uint spacing = 40;
    uint numberPointsWidth = 30;
    // decompose drawOffset since we'll be using them quite separately
    // will be updateing yPos frequently
    uint yPos = drawOffset.y + backButton->getDimensions().y + spacing;
    uint leftBorder = drawOffset.x;

    uint textStartX = leftBorder + numberPointsWidth;
    uint shortTextWidth = 400;
    uint longTextWidth = 600;
    uint buttonsStartX = textStartX + shortTextWidth + spacing;

    // "back" button

    backButton->draw(window);

    // 1. Copy payload

    drawStepNum(window, "1", font, vector2i(leftBorder, yPos));
    int textHeight = drawWrappedStepText(window, "Click to copy the signature payload:", font, vector2i(textStartX, yPos), shortTextWidth);

    // create copyButton if it's not already set.
    // this is kind of hacky, but I don't want to sort through this stateless/stateful dilemma right now.
    if (!copyButton)
    {
        copyButton = boost::shared_ptr<ImageButton>(new ImageButton(vector2i(buttonsStartX, yPos), COPYPASTE_BUTTON_DIMENSIONS, copyActionIcon));
    }
    copyButton->draw(window);

    int elementHeight = max(textHeight, COPYPASTE_BUTTON_DIMENSIONS.y);
    yPos += elementHeight + spacing;

    // 2. Go sign it

    drawStepNum(window, "2", font, vector2i(leftBorder, yPos));
    textHeight = drawWrappedStepText(window, "Sign the payload with an Ethereum/Web3 wallet. Metamask & myetherwallet.com/wallet/sign works well.", font, vector2i(textStartX, yPos), longTextWidth);

    yPos += textHeight + spacing;

    // 3. Paste signature

    drawStepNum(window, "3", font, vector2i(leftBorder, yPos));
    textHeight = drawWrappedStepText(window, "Click to paste the result here.", font, vector2i(textStartX, yPos), shortTextWidth);

    if (!pasteButton)
    {
        pasteButton = boost::shared_ptr<ImageButton>(new ImageButton(vector2i(buttonsStartX, yPos), COPYPASTE_BUTTON_DIMENSIONS, pasteActionIcon));
    }
    pasteButton->draw(window);

    elementHeight = max(textHeight, COPYPASTE_BUTTON_DIMENSIONS.y);
    yPos += elementHeight + spacing;
}
optional<LoginWindow::Msg> LoginWindow::processEvent(sf::Event event)
{
    switch (event.type)
    {
        case sf::Event::MouseMoved:
        {
            backButton->registerMouseMove(vector2i(event.mouseMove.x, event.mouseMove.y));
            if (copyButton) copyButton->registerMouseMove(vector2i(event.mouseMove.x, event.mouseMove.y));
            if (pasteButton) pasteButton->registerMouseMove(vector2i(event.mouseMove.x, event.mouseMove.y));
            break;
        }
        case sf::Event::MouseButtonPressed:
        {
            backButton->registerPress();
            if (copyButton) copyButton->registerPress();
            if (pasteButton) pasteButton->registerPress();
            break;
        }
        case sf::Event::MouseButtonReleased:
        {
            if (backButton->registerRelease())
            {
                return Back;
            }
            if (copyButton)
            {
                if (copyButton->registerRelease())
                {
                    sf::Clipboard::setString(sigChallenge);
                    if (sf::Clipboard::getString() == sigChallenge)
                    {
                        copyButton->changeImage(copyDoneIcon);
                    }
                    else
                    {
                        cout << "ERROR: copying to the clipboard didn't work!" << endl;
                    }
                }
            }
            if (pasteButton)
            {
                if (pasteButton->registerRelease())
                {
                    string inClipboard = sf::Clipboard::getString();
                    pasteButton->changeImage(pasteDoneIcon);
                    sigResponse = inClipboard;
                    sf::Clipboard::setString("");
                    return ResponseEntered;
                }
            }
            break;
        }
        default:
        {}
    }

    return {};
}


void drawStepNum(sf::RenderWindow* window, string numStr, sf::Font* font, vector2i drawPos)
{
    sf::Text text(numStr + ".", *font, 26);
    text.setFillColor(sf::Color::White);
    text.setPosition(toSFVecF(drawPos));
    window->draw(text);
}
uint drawWrappedStepText(sf::RenderWindow* window, string textString, sf::Font* font, vector2i drawPos, uint textBoxWidth)
{
    uint textHeight = GH::wrapAndRenderTextAtPos(window, textString, font, 24, sf::Color::White, textBoxWidth, drawPos);

    return textHeight;
}
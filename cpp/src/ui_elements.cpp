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
    sf::RectangleShape buttonRect(toSFVecF(vector2fl(p2 - p1)));
    buttonRect.setPosition(toSFVecF(vector2fl(p1)));

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

    renderedText.setPosition(toSFVecF(vector2fl(p1 + vector2i(offsetX, textOffsetY))));
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

    sprite.setPosition(toSFVecF(vector2fl(pos)));
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
    sf::RectangleShape mainBox(toSFVecF(vector2fl(p2 - p1)));
    mainBox.setPosition(toSFVecF(vector2fl(p1)));
    mainBox.setFillColor(sf::Color::Black);
    mainBox.setOutlineThickness(1);
    mainBox.setOutlineColor(sf::Color(0, 0, 255));
    window->draw(mainBox);

    drawContent(window, p1 + vector2i(WINDOW_PADDING, WINDOW_PADDING));
}

NoticeWindow::NoticeWindow(vector2i center, string message, sf::Font* font)
    : message(message), font(font)
{
    unsigned int spacing = 50;

    auto renderedText = renderText();
    unsigned int textHeight = renderedText.getLocalBounds().height;

    unsigned int totalHeight = textHeight + MAIN_MENU_BUTTON_HEIGHT + spacing + WINDOW_PADDING*2;

    vector2i halfDims = vector2i(NOTICE_WINDOW_WIDTH, totalHeight) / 2;
    p1 = center - halfDims;
    p2 = center + halfDims;

    unsigned int okayButtonY = p1.y + WINDOW_PADDING + textHeight + spacing;
    vector2i okayButtonPos(p1.x + WINDOW_PADDING, okayButtonY);
    okayButton = boost::shared_ptr<TextButton>(new TextButton(
        okayButtonPos,
        vector2i(NOTICE_WINDOW_WIDTH - (WINDOW_PADDING*2), MAIN_MENU_BUTTON_HEIGHT),
        "Okay",
        font,
        MAIN_MENU_BUTTON_FONT_SIZE,
        MAIN_MENU_BUTTON_TEXT_OFFSET_Y
    ));
}
sf::Text NoticeWindow::renderText()
{
    return GH::wrapTextBlock(message, font, NOTICE_WINDOW_FONT_SIZE, NOTICE_WINDOW_WIDTH - (WINDOW_PADDING*2));
}
void NoticeWindow::drawContent(sf::RenderWindow* window, vector2i drawOffset)
{
    auto renderedText = renderText();
    renderedText.setPosition(toSFVecF(drawOffset));
    renderedText.setFillColor(sf::Color::White);
    window->draw(renderedText);
    okayButton->draw(window);
}
bool NoticeWindow::processEvent(sf::Event event)
{
    switch (event.type)
    {
        case sf::Event::MouseMoved:
        {
            okayButton->registerMouseMove(vector2i(event.mouseMove.x, event.mouseMove.y));
            break;
        }
        case sf::Event::MouseButtonPressed:
        {
            okayButton->registerPress();
            break;
        }
        case sf::Event::MouseButtonReleased:
        {
            if (okayButton->registerRelease())
            {
                return true;
            }
            break;
        }
        default:
            break;
    }
    return false;
}

void runNoticeWindow(sf::RenderWindow* window, string message, sf::Font* font)
{
    vector2i center = getScreenSize(window) / 2;

    NoticeWindow noticeWindow(center, message, font);

    while (window->isOpen())
    {
        sf::Event event;
        while (window->pollEvent(event))
        {
            bool okayPressed = noticeWindow.processEvent(event);
            if (okayPressed)
            {
                return;
            }
        }

        window->clear(sf::Color::Black);
        noticeWindow.draw(window);
        window->display();
    }
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
unsigned int drawWrappedStepText(sf::RenderWindow* window, string textString, sf::Font* font, vector2i drawPos, unsigned int textBoxWidth);
unsigned int drawErrorText(sf::RenderWindow* window, string errorString, sf::Font* font, vector2i drawPos, unsigned int textBoxWidth);
void LoginWindow::drawContent(sf::RenderWindow* window, vector2i drawOffset)
{
    unsigned int spacing = 40;
    unsigned int numberPointsWidth = 30;
    // decompose drawOffset since we'll be using them quite separately
    // will be updateing yPos frequently
    unsigned int yPos = drawOffset.y + backButton->getDimensions().y + spacing;
    unsigned int leftBorder = drawOffset.x;

    unsigned int textStartX = leftBorder + numberPointsWidth;
    unsigned int shortTextWidth = 400;
    unsigned int longTextWidth = 500;
    unsigned int buttonsStartX = textStartX + shortTextWidth + spacing;

    // "back" button

    backButton->draw(window);

    // 1. Copy payload

    drawStepNum(window, "1", font, vector2i(leftBorder, yPos));
    int textHeight = drawWrappedStepText(window, "Click to copy the server's challenge text:", font, vector2i(textStartX, yPos), shortTextWidth);

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
    textHeight = drawWrappedStepText(window, "Sign the challenge text with the Login Helper at coinfight.io. (You'll need a web3 wallet like Metamask)", font, vector2i(textStartX, yPos), longTextWidth);

    yPos += textHeight + spacing;

    // 3. Paste signature

    drawStepNum(window, "3", font, vector2i(leftBorder, yPos));
    textHeight = drawWrappedStepText(window, "Click to paste the resulting signature here.", font, vector2i(textStartX, yPos), shortTextWidth);

    if (!pasteButton)
    {
        pasteButton = boost::shared_ptr<ImageButton>(new ImageButton(vector2i(buttonsStartX, yPos), COPYPASTE_BUTTON_DIMENSIONS, pasteActionIcon));
    }
    pasteButton->draw(window);

    elementHeight = max(textHeight, COPYPASTE_BUTTON_DIMENSIONS.y);
    yPos += elementHeight + spacing;

    // Display any error
    if (errorString)
    {
        drawErrorText(window, *errorString, font, vector2i(leftBorder, yPos), 500);
    }
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
                    if (inClipboard == "" || inClipboard == sigChallenge)
                    {
                        errorString = "You haven't copied the signature into your clipboard yet - see step 2.";
                        return {};
                    }

                    optional<tuple<Address, string>> maybeAddressAndSig = decodeAddressAndSig(inClipboard);
                    if (!maybeAddressAndSig)
                    {
                        errorString = "Unexpected format. Are you using coinfight.io to sign the message?";
                        return {};
                    }

                    pasteButton->changeImage(pasteDoneIcon);
                    addressAndSigResponse = maybeAddressAndSig;
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
unsigned int drawWrappedStepText(sf::RenderWindow* window, string textString, sf::Font* font, vector2i drawPos, unsigned int textBoxWidth)
{
    unsigned int textHeight = GH::wrapAndRenderTextAtPos(window, textString, font, 24, sf::Color::White, textBoxWidth, drawPos);

    return textHeight;
}
unsigned int drawErrorText(sf::RenderWindow* window, string errorString, sf::Font* font, vector2i drawPos, unsigned int textBoxWidth)
{
    unsigned int textHeight = GH::wrapAndRenderTextAtPos(window, errorString, font, 18, sf::Color::Red, textBoxWidth, drawPos);
    return textHeight;
}


GiftUnitsWindow::GiftUnitsWindow(vector2i center, sf::Font* font)
    : font(font)
{
    vector2i halfDims = GIFT_WINDOW_DIMENSIONS / 2;
    p1 = center - halfDims;
    p2 = center + halfDims;

    vector2i drawStart = p1 + vector2i(WINDOW_PADDING, WINDOW_PADDING);

    backButton = boost::shared_ptr<TextButton>(
        new TextButton(
            drawStart,
            vector2i(GIFT_WINDOW_DIMENSIONS.x - WINDOW_PADDING * 2, MAIN_MENU_BUTTON_HEIGHT),
            "Back",
            font,
            MAIN_MENU_BUTTON_FONT_SIZE,
            MAIN_MENU_BUTTON_TEXT_OFFSET_Y
        )
    );
}
void GiftUnitsWindow::drawContent(sf::RenderWindow* window, vector2i drawOffset)
{
    // lots of repeated stuff from LoginWindow...
    unsigned int spacing = 40;
    unsigned int textWidth = 400;

    unsigned int yPos = drawOffset.y + backButton->getDimensions().y + spacing;
    unsigned int buttonsStartX = drawOffset.x + textWidth + spacing;


    backButton->draw(window);

    int textHeight = GH::wrapAndRenderTextAtPos(
        window,
        "Copy the recipient's address, then click to paste here:",
        font,
        24,
        sf::Color::White,
        textWidth,
        vector2i(drawOffset.x, yPos)
    );

    if (!pasteButton)
    {
        pasteButton = boost::shared_ptr<ImageButton>(new ImageButton(vector2i(buttonsStartX, yPos), COPYPASTE_BUTTON_DIMENSIONS, pasteActionIcon));
    }
    pasteButton->draw(window);

    yPos += (max(COPYPASTE_BUTTON_DIMENSIONS.y, textHeight)) + spacing;

    textHeight = GH::wrapAndRenderTextAtPos(
        window,
        (pastedText ? *pastedText : " "),
        font,
        14,
        sf::Color(150, 150, 255),
        textWidth,
        vector2i(drawOffset.x, yPos)
    );

    yPos += textHeight + spacing;

    if (submittedAddress)
    {
        if (!submitButton)
        {
            submitButton = boost::shared_ptr<TextButton>(new TextButton(
                vector2i(drawOffset.x, yPos),
                vector2i(GIFT_WINDOW_DIMENSIONS.x - WINDOW_PADDING * 2, MAIN_MENU_BUTTON_HEIGHT),
                "Submit",
                font,
                MAIN_MENU_BUTTON_FONT_SIZE,
                MAIN_MENU_BUTTON_TEXT_OFFSET_Y
            ));
        }
        submitButton->draw(window);
    }

    // Display any error
    if (errorString)
    {
        drawErrorText(window, *errorString, font, vector2i(drawOffset.x, yPos), textWidth);
    }
}
optional<GiftUnitsWindow::Msg> GiftUnitsWindow::processEvent(sf::Event event)
{
    switch (event.type)
    {
        case sf::Event::MouseMoved:
        {
            backButton->registerMouseMove(vector2i(event.mouseMove.x, event.mouseMove.y));
            if (submitButton) submitButton->registerMouseMove(vector2i(event.mouseMove.x, event.mouseMove.y));
            if (pasteButton) pasteButton->registerMouseMove(vector2i(event.mouseMove.x, event.mouseMove.y));
            break;
        }
        case sf::Event::MouseButtonPressed:
        {
            backButton->registerPress();
            if (submitButton) submitButton->registerPress();
            if (pasteButton) pasteButton->registerPress();
            break;
        }
        case sf::Event::MouseButtonReleased:
        {
            if (backButton->registerRelease())
            {
                return Back;
            }
            if (pasteButton)
            {
                if (pasteButton->registerRelease())
                {
                    string inClipboard = sf::Clipboard::getString();
                    if (inClipboard == "")
                    {
                        return {};
                    }

                    pastedText = inClipboard;
                    optional<Address> maybeAddress = validateAddress(inClipboard);
                    if (!maybeAddress)
                    {
                        errorString = "I can't parse that address....";
                        return {};
                    }

                    errorString = {};
                    pasteButton->changeImage(pasteDoneIcon);
                    submittedAddress = maybeAddress;
                    return {};
                }
            }
            if (submitButton) 
            {
                if (submitButton->registerRelease())
                {
                    if (submittedAddress)
                    {
                        return AddressSubmitted;
                    }
                }
                break;
            }
        }
        default:
        {}
    }

    return {};
}



UXBox::UXBox(vector2i upperLeft, vector2i size)
    : upperLeft(upperLeft), size(size)
{}

void UXBox::draw(sf::RenderWindow* window)
{
    sf::RectangleShape mainBox(toSFVecF(size));
    mainBox.setPosition(toSFVecF(upperLeft));
    mainBox.setFillColor(UX_BOX_BACKGROUND_COLOR);
    mainBox.setOutlineThickness(1);
    mainBox.setOutlineColor(UX_BOX_BORDER_COLOR);
    window->draw(mainBox);

    drawContent(window, upperLeft + UX_BOX_PADDING);
}

KeyButton::KeyButton(vector2i upperLeft, sf::Keyboard::Key key, sf::Text keyCharText)
    : Button(upperLeft, upperLeft + KEYBUTTON_SIZE), key(key), keyCharText(keyCharText)
{}

void KeyButton::setKeyButtonActionInfo(optional<KeyButtonActionInfo> _actionInfo)
{
    maybeActionInfo = _actionInfo;
}

void KeyButton::draw(sf::RenderWindow* window)
{
    sf::RectangleShape mainBox(toSFVecF(KEYBUTTON_SIZE));
    mainBox.setPosition(toSFVecF(p1));
    mainBox.setOutlineColor(
        (bool)(maybeActionInfo) ?
        sf::Color(100, 100, 255) :
        sf::Color(80, 80, 80)
    );
    mainBox.setFillColor(
        (bool)(maybeActionInfo) ?
        sf::Color(100, 100, 255, 100) :
        sf::Color::Black
    );
    window->draw(mainBox);

    // keyCharText = sf::Text("s", )
    keyCharText.setPosition(toSFVecF(p1));
    keyCharText.setFillColor(
        (bool)(maybeActionInfo) ?
        sf::Color::White :
        sf::Color(80, 80, 80)
    );
    window->draw(keyCharText);

    if (maybeActionInfo)
    {
        auto actionInfo = *maybeActionInfo;

        actionInfo.sprite.setPosition(toSFVecF(p1 + KEYBUTTON_PADDING));

        window->draw(actionInfo.sprite);
    }
}

// Not needed since we're overriding draw() above
void KeyButton::drawContent(sf::RenderWindow* window) {}

KeyButtonUXBox::KeyButtonUXBox(vector2i upperLeft, sf::Font* font)
    : UXBox(upperLeft, upperLeft + KEYBUTTONBOX_SIZE)
{
    KeyButton testKey(upperLeft + vector2i(20, 20), sf::Keyboard::G, sf::Text('G', *font, 18));
    KeyButtonActionInfo actionInfo;
    actionInfo.keyButtonMsg = WarpInGateway;
    
    if (!t.create(40, 40))
    {
        cout << "whaaaaaaa" << endl;
    }
    t.clear(sf::Color(255, 0, 0));
    t.display();
    actionInfo.sprite = sf::Sprite(t.getTexture());
    testKey.setKeyButtonActionInfo({actionInfo});
    keyButtons.push_back(testKey);
}

void KeyButtonUXBox::drawContent(sf::RenderWindow* window, vector2i upperLeft)
{
    for (unsigned int i=0; i<keyButtons.size(); i++)
    {
        keyButtons[i].draw(window);
    }
}
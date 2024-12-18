#include <iostream>
#include <sstream>
#include "ui_elements.h"
#include "common.h"
#include "myvectors.h"
#include "graphics_helpers.h"

using namespace std;

sf::Texture
    copyActionSource,
    copyDoneSource,
    pasteActionSource,
    pasteDoneSource,
    ckbSymbolSource
;

sf::Sprite
    copyActionIcon,
    copyDoneIcon,
    pasteActionIcon,
    pasteDoneIcon,
    ckbSymbolIcon
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

    ckbSymbolSource.setSmooth(true);
    if (!ckbSymbolSource.loadFromFile("nervos-n-white.png"))
    {
        throw runtime_error("Can't load CKB symbol icon");
    }
    ckbSymbolIcon.setTexture(ckbSymbolSource);
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

sf::FloatRect displayCurrencyAmount(sf::RenderWindow* window, coinsInt amount, sf::Font* font, int size, sf::Color color, vector2fl drawPos, vector2fl drawOriginVector)
{
    sf::Text amountText(sf::String(coinsIntToDollarString(amount)), *font, size);

    // float originalSymbolHeight = ckbSymbolIcon.getLocalBounds().height;
    // float neededScale = (((float)size) / (float)originalSymbolHeight) * 0.65;
    // ckbSymbolIcon.setScale(neededScale, neededScale);

    float spacing = size * 0.4;

    float totalWidth = amountText.getLocalBounds().width;// + spacing + ckbSymbolIcon.getGlobalBounds().width;
    float totalHeight = amountText.getLocalBounds().height;

    // drawOriginVector determines where this is drawn "from":
    // (0,0) indicates upper left while i.e. (0.5, 1) indicates middle bottom
    // Given this and our width and height, we need to calculate our actual upperLeft point to start drawing from.
    vector2fl drawUpperLeftOffset(
        totalWidth * drawOriginVector.x,
        totalHeight * drawOriginVector.y
    );
    vector2fl drawUpperLeft = drawPos - drawUpperLeftOffset;
    
    vector2fl symbolUpperLeft = drawUpperLeft + vector2fl(amountText.getLocalBounds().width + spacing, (size * 0.35));

    amountText.setPosition(toSFVecF(drawUpperLeft));
    // ckbSymbolIcon.setPosition(toSFVecF(symbolUpperLeft));

    amountText.setFillColor(color);
    // ckbSymbolIcon.setColor(color);

    window->draw(amountText);
    // window->draw(ckbSymbolIcon);

    return sf::FloatRect(drawUpperLeft.x, drawUpperLeft.y, totalWidth, totalHeight);
}

// void displayCKBSymbol(sf::RenderWindow* window, int height, sf::Color color, vector2fl drawPos, vector2fl drawOriginVector)
// {
//     float originalSymbolHeight = ckbSymbolIcon.getLocalBounds().height;
//     float neededScale = (((float)height) / (float)originalSymbolHeight);
//     ckbSymbolIcon.setScale(neededScale, neededScale);

//     vector2fl drawUpperLeftOffset(
//         ckbSymbolIcon.getGlobalBounds().width * drawOriginVector.x,
//         ckbSymbolIcon.getGlobalBounds().height * drawOriginVector.y
//     );
//     vector2fl drawUpperLeft = drawPos - drawUpperLeftOffset;

//     ckbSymbolIcon.setPosition(toSFVecF(drawUpperLeft));
//     ckbSymbolIcon.setColor(color);

//     window->draw(ckbSymbolIcon);
// }

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

bool UXBox::pointCollides(vector2i pos)
{
    return (
        upperLeft.x <= pos.x &&
        upperLeft.y <= pos.y &&
        pos.x <= upperLeft.x + size.x &&
        pos.y <= upperLeft.y + size.y
    );
}

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

KeyButtonActionInfo::KeyButtonActionInfo(sf::Sprite* sprite, KeyButtonHintInfo hintInfo, KeyButtonMsg keyButtonMsg)
    : sprite(sprite), hintInfo(hintInfo), keyButtonMsg(keyButtonMsg) {}

KeyButton::KeyButton(vector2i upperLeft, sf::Keyboard::Key key, sf::Text keyCharText)
    : Button(upperLeft, upperLeft + KEYBUTTON_SIZE), key(key), keyCharText(keyCharText), active(false)
{}

void KeyButton::setKeyButtonActionInfo(optional<KeyButtonActionInfo> _actionInfo)
{
    maybeActionInfo = _actionInfo;
}

void KeyButton::draw(sf::RenderWindow* window)
{
    sf::Color
        backgroundColor,
        outlineColor,
        keyColor
    ;

    sf::Color gray(80, 80, 80);
    sf::Color activeRedBackground(150, 0, 0, 100);
    sf::Color activeOutlineColor(100, 100, 255);
    sf::Color mouseoverBackgroundColor(100, 100, 255, 100);
    sf::Color pressedBackgroundColor(100, 100, 255, 200);

    if (!maybeActionInfo)
    {
        backgroundColor = sf::Color::Black;
        outlineColor = gray;
        keyColor = gray;
    }
    else if (visualFlashClock.getElapsedTime() < sf::seconds(0.06) || active)
    {
        backgroundColor = activeRedBackground;
        outlineColor = activeOutlineColor;
        keyColor = sf::Color::White;
    }
    else // eligible but not active
    {
        outlineColor = activeOutlineColor;
        keyColor = sf::Color::White;
        if (pressed)
        {
            backgroundColor = pressedBackgroundColor;
        }
        else if (mouseover)
        {
            backgroundColor = mouseoverBackgroundColor;
        }
        else
        {
            backgroundColor = sf::Color::Black;
        }
    }

    sf::RectangleShape mainBox(toSFVecF(KEYBUTTON_SIZE));
    mainBox.setPosition(toSFVecF(p1));
    mainBox.setOutlineThickness(2);
    mainBox.setOutlineColor(outlineColor);
    mainBox.setFillColor(backgroundColor);
    window->draw(mainBox);

    if (maybeActionInfo)
    {
        auto actionInfo = *maybeActionInfo;

        actionInfo.sprite->setPosition(toSFVecF(p1 + KEYBUTTON_PADDING));

        window->draw(*actionInfo.sprite);
    }

    // keyCharText = sf::Text("s", )
    keyCharText.setPosition(toSFVecF(p1));
    keyCharText.setFillColor(keyColor);
    window->draw(keyCharText);
}

// Not needed since we're overriding draw() above
void KeyButton::drawContent(sf::RenderWindow* window) {}

KeyButton* KeyButtonUXBox::getKeyButton(sf::Keyboard::Key key)
{
    for (unsigned int i=0; i<keyButtons.size(); i++)
    {
        if (keyButtons[i].key == key)
        {
            return &keyButtons[i];
        }
    }
    return NULL;
}

KeyButtonUXBox::KeyButtonUXBox(vector2i upperLeft, sf::Font* font, sf::Sprite* (*getSpriteForMsg)(KeyButtonMsg))
    : UXBox(upperLeft, KEYBUTTONBOX_SIZE), getSpriteForMsg(getSpriteForMsg)
{
    vector<tuple<sf::Keyboard::Key, char>> keyButtonSeeds =
    {
        {sf::Keyboard::Q, 'Q'},
        {sf::Keyboard::W, 'W'},
        {sf::Keyboard::E, 'E'},
        {sf::Keyboard::R, 'R'},
        {sf::Keyboard::A, 'A'},
        {sf::Keyboard::S, 'S'},
        {sf::Keyboard::D, 'D'},
        {sf::Keyboard::F, 'F'}
    };

    for (unsigned int i=0; i<keyButtonSeeds.size(); i++)
    {
        int rowNum = i / 4;
        int rowOffsetX = (rowNum == 0 ? 0 : KEYBUTTON_ROW2_OFFSETX);

        int xOffset = (i % 4) * (KEYBUTTON_SIZE.x + KEYBUTTON_SPACING) + rowOffsetX;
        int yOffset = (i / 4) * (KEYBUTTON_SIZE.y + KEYBUTTON_SPACING);

        vector2i buttonPos = upperLeft + KEYBUTTONBOX_PADDING + vector2i(xOffset, yOffset);

        auto key = get<0>(keyButtonSeeds[i]);

        sf::Text keyText(get<1>(keyButtonSeeds[i]), *font, 18);

        keyButtons.push_back(KeyButton(buttonPos, key, keyText));
    }
}

void KeyButtonUXBox::clearActionInfos()
{
    for (unsigned int i=0; i<keyButtons.size(); i++)
    {
        keyButtons[i].maybeActionInfo = {};
    }
}

void KeyButtonUXBox::setUnitCmdOrThrow(sf::Keyboard::Key key, KeyButtonHintInfo hintInfo, KeyButtonMsg keyButtonMsg)
{
    if (auto keyButton = getKeyButton(key))
    {
        keyButton->setKeyButtonActionInfo(
            KeyButtonActionInfo(
                getSpriteForMsg(keyButtonMsg),
                hintInfo,
                keyButtonMsg
            )
        );
    }
    else
    {
        throw runtime_error("Can't find keybutton with that key");
    }
}

void KeyButtonUXBox::returnToDefaultState()
{
    for (unsigned int i=0; i<keyButtons.size(); i++)
    {
        keyButtons[i].active = false;
    }
}

optional<KeyButtonHintInfo> KeyButtonUXBox::getMouseoverHintInfo()
{
    for (unsigned int i=0; i<keyButtons.size(); i++)
    {
        if (auto actionInfo = keyButtons[i].maybeActionInfo)
        {
            if (keyButtons[i].mouseover)
            {
                return actionInfo->hintInfo;
            }
        }
    }
    
    return {};
}

bool KeyButtonUXBox::registerMouseMove(vector2i pos)
{
    for (unsigned int i=0; i<keyButtons.size(); i++)
    {
        if (keyButtons[i].maybeActionInfo)
        {
            keyButtons[i].registerMouseMove(pos);
        }
    }

    return this->pointCollides(pos);
}
bool KeyButtonUXBox::registerPress(vector2i pos)
{
    for (unsigned int i=0; i<keyButtons.size(); i++)
    {
        if (keyButtons[i].maybeActionInfo)
        {
            keyButtons[i].registerPress();
        }
    }

    return this->pointCollides(pos);
}
tuple<bool, optional<tuple<KeyButton*, KeyButtonMsg>>> KeyButtonUXBox::registerRelease(vector2i pos)
{
    bool pointCollides = this->pointCollides(pos);

    for (unsigned int i=0; i<keyButtons.size(); i++)
    {
        if (keyButtons[i].maybeActionInfo)
        {
            if (keyButtons[i].registerRelease())
            {
                return {pointCollides, {{&keyButtons[i], keyButtons[i].maybeActionInfo->keyButtonMsg}}};
            }
        }
    }

    return {pointCollides, {}};
}

optional<tuple<KeyButton*, KeyButtonMsg>> KeyButtonUXBox::handleKey(sf::Keyboard::Key key)
{
    if (auto keyButton = getKeyButton(key))
    {
        if (keyButton->maybeActionInfo)
        {
            keyButton->visualFlashClock.restart();
            return {{keyButton, keyButton->maybeActionInfo->keyButtonMsg}};
        }
    }
    return {};
}

void KeyButtonUXBox::drawContent(sf::RenderWindow* window, vector2i upperLeft)
{
    for (unsigned int i=0; i<keyButtons.size(); i++)
    {
        keyButtons[i].draw(window);
    }
}

UnitInfoUXBox::UnitInfoUXBox(vector2i upperLeft, sf::Font* font, vector<boost::shared_ptr<Unit>>* selectedUnits, sf::Sprite* (*getSpriteForUnitTypechar)(uint8_t))
    : UXBox(upperLeft, UNITINFOBOX_SIZE), font(font), selectedUnits(selectedUnits), getSpriteForUnitTypechar(getSpriteForUnitTypechar)
    {}

void displayUnitName(sf::RenderWindow* window, boost::shared_ptr<Unit> unit, sf::Font* font, vector2i upperLeftDrawPos)
{
    string name = unit->getTypename();
    sf::Text nameText(name, *font, 22);
    nameText.setPosition(toSFVecF(upperLeftDrawPos));

    window->draw(nameText);
}

const int HEALTH_TEXT_SIZE = 20;

sf::Color getHealthStringColor(boost::shared_ptr<Unit> unit)
{
    float healthRatio = (float)(unit->getEffectiveHealth()) / (float)(unit->getMaxHealth());
    if (healthRatio < 0.25)
    {
        return sf::Color(255, 0, 0);
    }
    else if (healthRatio < 0.6)
    {
        return sf::Color(255, 255, 0);
    }
    else
    {
        return sf::Color(0, 255, 0);
    }
}

void displayUnitHealth(sf::RenderWindow* window, boost::shared_ptr<Unit> unit, sf::Font* font, vector2i upperRightDrawPos)
{
    string currentHealthString = uint16ToString(unit->getEffectiveHealth());
    string maxHealthString = uint16ToString(unit->getMaxHealth());
    
    sf::Text firstHalf(currentHealthString, *font, HEALTH_TEXT_SIZE);
    firstHalf.setFillColor(getHealthStringColor(unit));

    sf::Text secondHalf(string(" / ") + maxHealthString, *font, HEALTH_TEXT_SIZE);
    secondHalf.setFillColor(sf::Color(255, 255, 255));

    vector2i drawPos = upperRightDrawPos;

    // align the whole thing right, by rendering the two pieces in reverse order
    drawPos -= vector2i(secondHalf.getLocalBounds().width, 0);
    secondHalf.setPosition(toSFVecF(drawPos));

    drawPos -= vector2i(firstHalf.getLocalBounds().width, 0);
    firstHalf.setPosition(toSFVecF(drawPos));

    window->draw(secondHalf);
    window->draw(firstHalf);
}

void displayUnitArt(sf::RenderWindow* window, vector2i upperLeft, uint8_t type, sf::Font* font)
{
    sf::RectangleShape border(toSFVecF(UNIT_ART_SIZE));
    border.setFillColor(sf::Color(0, 0, 100));
    border.setOutlineThickness(1);
    border.setOutlineColor(sf::Color(0, 0, 255));
    border.setPosition(toSFVecF(upperLeft));

    window->draw(border);

    sf::Text placeholder("wen art tho", *font, 12);
    placeholder.setOrigin(toSFVecF(vector2i(placeholder.getLocalBounds().width, placeholder.getLocalBounds().height) / 2));
    placeholder.setPosition(toSFVecF(upperLeft + (UNIT_ART_SIZE / 2)));
    placeholder.setFillColor(sf::Color(100, 100, 200));

    window->draw(placeholder);
}

void displayWorkOrderWarning(sf::RenderWindow* window, sf::Font* font, vector2i upperLeft, string warning)
{
    sf::Text warningText(warning, *font, 16);
    warningText.setPosition(toSFVecF(upperLeft));
    warningText.setFillColor(sf::Color::Red);
    window->draw(warningText);
}

int displayWorkOrderInfo(sf::RenderWindow* window, sf::Font* font, vector2i upperLeft, string mainJobName, optional<float> maybeMainJobCompletion, int numAdditionalJobs, optional<coinsInt> maybeFundsLeft, string additionalJobsLabel, sf::Color mainColor, bool active)
{
    int yOffset = 0;

    string jobText = mainJobName;
    if (maybeMainJobCompletion)
    {
        jobText += " (" + floatToShortPercentString(*maybeMainJobCompletion) + ")";
    }

    sf::Text nameText(jobText, *font, 16);
    nameText.setPosition(toSFVecF(upperLeft + vector2i(0, yOffset)));
    nameText.setFillColor(active ? mainColor : sf::Color(150, 150, 150));
    if (active) nameText.setStyle(sf::Text::Bold);
    window->draw(nameText);

    yOffset += nameText.getLocalBounds().height + 8;

    if (numAdditionalJobs > 0)
    {
        vector2i drawPos = upperLeft + vector2i(10, yOffset);

        stringstream ss;
        ss << "+ " << numAdditionalJobs << " more " << additionalJobsLabel << " jobs queued";
        sf::Text additionalJobsText(ss.str(), *font, 14);
        additionalJobsText.setPosition(toSFVecF(drawPos));
        additionalJobsText.setFillColor(sf::Color(150, 150, 150));
        window->draw(additionalJobsText);
    }

    if (maybeFundsLeft)
    {
        vector2i drawPos = upperLeft + vector2i(250, yOffset);

        displayCurrencyAmount(window, *maybeFundsLeft, font, 14, sf::Color::Yellow, drawPos, vector2fl(0, 0));

        drawPos += vector2i(50, 2);
        sf::Text left(" left", *font, 12);
        left.setPosition(toSFVecF(drawPos));
        left.setFillColor(sf::Color::White);
        window->draw(left);
    }

    yOffset += UNIT_INFO_STATUS_ELEMENT_SPACING + 12;

    return yOffset;
}

void displayPrimeGoldSourceInfo(sf::RenderWindow* window, sf::Font* font, vector2i upperLeft, boost::shared_ptr<Prime> prime)
{
    sf::Color nameColor(255, 255, 150);

    if (prime->scavengeTargetQueue.size() == 0)
    {
        if (auto fundsSource = prime->fundsSource)
        {
            auto entity = maybeEntityRefToPtrOrNull(*prime->getGameOrThrow(), fundsSource);
            if (!entity)
            {
                displayWorkOrderWarning(window, font, upperLeft, "???");
                return;
            }

            string sourceJobName;
            bool sourceJobIsActive;

            if (entity->typechar() == GATEWAY_TYPECHAR)
            {
                sourceJobName = "Withdrawing gold via Gateway";
            }
            else if (entity->typechar() == PRIME_TYPECHAR)
            {
                sourceJobName = "Withdrawing gold from a Prime";
            }
            else
            {
                displayWorkOrderWarning(window, font, upperLeft, "???");
                return;
            }

            if (auto moveTarget = prime->getMaybeMoveTarget())
            {
                sourceJobIsActive = (*moveTarget == Target(*fundsSource));
            }
            else
            {
                sourceJobIsActive = (bool)get<0>(prime->goldFlowFrom_view);
            }

            displayWorkOrderInfo(window, font, upperLeft, sourceJobName, {}, 0, {}, "source", nameColor, sourceJobIsActive);
            return;
        }
        else
        {
            displayWorkOrderWarning(window, font, upperLeft, "No gold sources assigned!");
            return;
        }
    }
    else // scavenge queue size > 0
    {
        string sourceJobName;
        optional<float> maybeSourceJobCompletion;
        bool sourceJobIsActive;

        coinsInt fundsInQueue = 0;
        for (unsigned int i=0; i<prime->scavengeTargetQueue.size(); i++)
        {
            if (auto entity = maybeEntityRefToPtrOrNull(*prime->getGameOrThrow(), prime->scavengeTargetQueue[i].castToEntityRef()))
            {
                if (auto goldpile = boost::dynamic_pointer_cast<GoldPile, Entity>(entity))
                {
                    fundsInQueue += goldpile->gold.getInt();
                }
                else if (auto unit = boost::dynamic_pointer_cast<Unit, Entity>(entity))
                {
                    fundsInQueue += unit->getBuilt();

                    if (auto prime = boost::dynamic_pointer_cast<Prime, Unit>(unit))
                    {
                        fundsInQueue += prime->heldGold.getInt();
                    }
                }
            }
        }

        if (prime->scavengeTargetQueue[0].type == Target::PointTarget)
        {
            sourceJobName = "Scavenging";
            if (auto moveTarget = prime->getMaybeMoveTarget())
            {
                if (prime->scavengeTargetQueue[0] == *moveTarget)
                {
                    sourceJobIsActive = true;
                }
                else
                {
                    if (auto fetchTarget = prime->fetchToImmediateTarget)
                    {
                        sourceJobIsActive = (prime->fetchToImmediateTarget == moveTarget->castToEntityRef());
                    }
                    else
                    {
                        sourceJobIsActive = false;
                    }
                }
            }
        }
        else if (auto entity = prime->scavengeTargetQueue[0].castToEntityPtr(*prime->getGameOrThrow()))
        {
            sourceJobIsActive = (prime->scavengeTargetQueue[0] == prime->getMaybeMoveTarget());
            if (entity->typechar() == GOLDPILE_TYPECHAR)
            {
                sourceJobName = "Picking up gold";
            }
            else if (auto unit = boost::dynamic_pointer_cast<Unit, Entity>(entity))
            {
                sourceJobName = "Scuttling " + unit->getTypename();
                maybeSourceJobCompletion = 1 - unit->getBuiltRatio();
            }
        }

        if (get<0>(prime->goldFlowFrom_view))
        {
            sourceJobIsActive = true;
        }

        if (prime->heldGold.getSpaceLeft() == 0)
        {
            sourceJobIsActive = false;
        }

        displayWorkOrderInfo(window, font, upperLeft, sourceJobName, maybeSourceJobCompletion, prime->scavengeTargetQueue.size() - 1, fundsInQueue, "source", nameColor, sourceJobIsActive);
        return;
    }
}

void displayPrimeGoldDestInfo(sf::RenderWindow* window, sf::Font* font, vector2i upperLeft, boost::shared_ptr<Prime> prime)
{
    sf::Color nameColor(150, 150, 255);

    if (prime->buildTargetQueue.size() == 0)
    {
        if (auto fundsDest = prime->fundsDest)
        {
            string destJobName;
            optional<float> maybeDestJobCompletion;
            bool destJobIsActive;

            auto destEntity = maybeEntityRefToPtrOrNull(*prime->getGameOrThrow(), *fundsDest);
            if (!destEntity)
            {
                displayWorkOrderWarning(window, font, upperLeft, "???");
                return;
            }

            if (destEntity->typechar() == GATEWAY_TYPECHAR)
            {
                destJobName = "Depositing gold through Gateway";
            }
            else if (destEntity->typechar() == GOLDPILE_TYPECHAR)
            {
                destJobName = "Depositing to gold pile";
            }
            else if (destEntity->typechar() == PRIME_TYPECHAR)
            {
                destJobName = "Depositing gold to a Prime";
            }
            else
            {
                displayWorkOrderWarning(window, font, upperLeft, "???");
                return;
            }

            if (auto moveTarget = prime->getMaybeMoveTarget())
            {
                destJobIsActive = (*moveTarget == Target(*fundsDest));
            }
            else
            {
                destJobIsActive = false;
            }

            if (get<0>(prime->goldFlowTo_view))
            {
                destJobIsActive = true;
            }

            displayWorkOrderInfo(window, font, upperLeft, destJobName, {}, 0, {}, "build", nameColor, destJobIsActive);
            return;
        }
        else
        {
            displayWorkOrderWarning(window, font, upperLeft, "No build/deposit jobs assigned!");
            return;
        }
    }
    else // build queue size > 0
    {
        auto targetEntity = maybeEntityRefToPtrOrNull(*prime->getGameOrThrow(), prime->buildTargetQueue[0]);
        if (!targetEntity)
        {
            displayWorkOrderWarning(window, font, upperLeft, "???");
            return;
        }
        auto targetUnit = boost::dynamic_pointer_cast<Unit, Entity>(targetEntity);
        if (!targetUnit)
        {
            displayWorkOrderWarning(window, font, upperLeft, "???");
            return;
        }

        string buildJobName = string("Building ") + targetUnit->getTypename();
        float buildJobCompletion = targetUnit->getBuiltRatio();

        bool buildJobIsActive = false;
        if (auto moveTarget = prime->getMaybeMoveTarget())
        {
            if (*moveTarget == Target(prime->buildTargetQueue[0]))
            {
                buildJobIsActive = true;
            }
        }

        if (get<0>(prime->goldFlowTo_view))
        {
            buildJobIsActive = true;
        }

        coinsInt fundsLeft = 0;
        for (unsigned int i=0; i<prime->buildTargetQueue.size(); i++)
        {
            if (auto unit = boost::dynamic_pointer_cast<Unit, Entity>(maybeEntityRefToPtrOrNull(*prime->getGameOrThrow(), prime->buildTargetQueue[i])))
            {
                fundsLeft += (unit->getCost() - unit->getBuilt());
            }
        }

        displayWorkOrderInfo(window, font, upperLeft, buildJobName, buildJobCompletion, prime->buildTargetQueue.size() - 1, fundsLeft, "build", nameColor, buildJobIsActive);
        return;
    }
}

void displayPrimeStatus(sf::RenderWindow* window, sf::Font* font, vector2i upperLeft, int availableWidth, boost::shared_ptr<Prime> prime)
{
    int yOffset = 0;

    vector2fl currencyNumeratorDrawPos = upperLeft + vector2i(0, yOffset);
    sf::Rect currencyNumeratorDrawRect = displayCurrencyAmount(window, prime->heldGold.getInt(), font, 16, sf::Color::Yellow, currencyNumeratorDrawPos, vector2fl(0, 0));

    sf::Text slashText(" / ", *font, 16);
    slashText.setPosition(toSFVecF(currencyNumeratorDrawPos + vector2fl(30, 0)));
    slashText.setFillColor(sf::Color(100, 100, 100));
    window->draw(slashText);
    
    vector2fl currencyDenominatorDrawPos = currencyNumeratorDrawPos + vector2fl(50, 0);
    sf::Rect currencyDenominatorDrawRect = displayCurrencyAmount(window, prime->heldGold.max, font, 16, sf::Color(100, 100, 100), currencyDenominatorDrawPos, vector2fl(0, 0));

    sf::Text percentText(" (" + floatToShortPercentString(prime->getHeldGoldRatio()) + ")", *font, 16);
    percentText.setPosition(sf::Vector2f(currencyDenominatorDrawRect.left + currencyDenominatorDrawRect.width, currencyDenominatorDrawRect.top));
    percentText.setFillColor(sf::Color::White);
    window->draw(percentText);

    yOffset += percentText.getLocalBounds().height + 30;

    displayPrimeGoldSourceInfo(window, font, upperLeft + vector2i(0, yOffset), prime);

    yOffset += 50;

    displayPrimeGoldDestInfo(window, font, upperLeft + vector2i(0, yOffset), prime);
}

void displayGatewayGoldSpendInfo(sf::RenderWindow* window, sf::Font* font, vector2i upperLeft, boost::shared_ptr<Gateway> gateway)
{
    sf::Color nameColor(255, 150, 150);
    coinsInt fundsLeftInQueue = 0;
    for (unsigned int i=0; i<gateway->buildTargetQueue.size(); i++)
    {
        if (auto unit = boost::dynamic_pointer_cast<Unit, Entity>(maybeEntityRefToPtrOrNull(*gateway->getGameOrThrow(), gateway->buildTargetQueue[i])))
        {
            fundsLeftInQueue += (unit->getCost() - unit->getBuilt());
        }
    }

    if (gateway->maybeWithdrawingPrime)
    {
        displayWorkOrderInfo(window, font, upperLeft, "Funding Prime", {}, gateway->buildTargetQueue.size(), fundsLeftInQueue, "build", nameColor, true);
        return;
    }
    else if (auto depositTarget = gateway->getMaybeDepositTarget())
    {
        string spendJobName;
        optional<float> jobCompletion;

        auto entity = maybeEntityRefToPtrOrNull(*gateway->getGameOrThrow(), get<0>(*depositTarget));
        if (!entity)
        {
            displayWorkOrderWarning(window, font, upperLeft, "???");
            return;
        }

        if (entity->typechar() == GOLDPILE_TYPECHAR)
        {
            spendJobName = "Depositing to gold pile";
        }
        else if (auto unit = boost::dynamic_pointer_cast<Unit, Entity>(entity))
        {
            if (!unit->isFullyBuilt())
            {
                spendJobName = string("Building ") + unit->getTypename();
                jobCompletion = unit->getBuiltRatio();
            }
            else if (unit->typechar() == PRIME_TYPECHAR)
            {
                spendJobName = "Funding Prime";
            }
            else
            {
                displayWorkOrderWarning(window, font, upperLeft, "???");
                return;
            }
        }
        else
        {
            displayWorkOrderWarning(window, font, upperLeft, "???");
            return;
        }

        bool isActive = (bool)get<0>(gateway->goldFlowTo_view);
        displayWorkOrderInfo(window, font, upperLeft, spendJobName, jobCompletion, gateway->buildTargetQueue.size() - 1, fundsLeftInQueue, "build", nameColor, isActive);
        return;
    }
    else
    {
        displayWorkOrderInfo(window, font, upperLeft, "No build jobs", {}, 0, {}, "build", sf::Color(150, 150, 150), false);
    }
}

void displayGatewayGoldIncomeInfo(sf::RenderWindow* window, sf::Font* font, vector2i upperLeft, boost::shared_ptr<Gateway> gateway)
{
    sf::Color nameColor(150, 255, 150);

    coinsInt fundsLeftInQueue = 0;
    for (unsigned int i=0; i<gateway->scuttleTargetQueue.size(); i++)
    {
        if (auto entity = maybeEntityRefToPtrOrNull(*gateway->getGameOrThrow(), gateway->scuttleTargetQueue[i]))
        {
            if (auto goldpile = boost::dynamic_pointer_cast<GoldPile, Entity>(entity))
            {
                fundsLeftInQueue += goldpile->gold.getInt();
            }
            if (auto unit = boost::dynamic_pointer_cast<Unit, Entity>(entity))
            {
                if (auto prime = boost::dynamic_pointer_cast<Prime, Unit>(unit))
                {
                    fundsLeftInQueue += prime->heldGold.getInt();
                }
                fundsLeftInQueue += unit->getBuilt();
            }
        }
    }

    if (gateway->maybeDepositingPrime)
    {
        if (auto prime = boost::dynamic_pointer_cast<Prime, Entity>(maybeEntityRefToPtrOrNull(*gateway->getGameOrThrow(), gateway->maybeDepositingPrime)))
        {
            fundsLeftInQueue += prime->heldGold.getInt();
            displayWorkOrderInfo(window, font, upperLeft, "Extracting from Prime", {}, gateway->scuttleTargetQueue.size(), fundsLeftInQueue, "extraction", nameColor, true);
            return;
        }
    }
    else if (auto scuttleTarget = gateway->getMaybeScuttleTarget())
    {
        string incomeJobName;
        optional<float> jobCompletion;

        auto entity = maybeEntityRefToPtrOrNull(*gateway->getGameOrThrow(), get<0>(*scuttleTarget));
        if (!entity)
        {
            displayWorkOrderWarning(window, font, upperLeft, "???");
            return;
        }

        if (entity->typechar() == GOLDPILE_TYPECHAR)
        {
            incomeJobName = "Extracting gold";
        }
        else if (auto unit = boost::dynamic_pointer_cast<Unit, Entity>(entity))
        {
            if (auto prime = boost::dynamic_pointer_cast<Prime, Unit>(unit))
            {
                if (prime->isFullyBuilt())
                {
                    incomeJobName = "Extracting from Prime";
                }
                else
                {
                    incomeJobName = "Scuttling Prime";
                }
            }
            else
            {
                incomeJobName = string("Scuttling ") + unit->getTypename();
                jobCompletion = 1 - unit->getBuiltRatio();
            }
        }
        else
        {
            displayWorkOrderWarning(window, font, upperLeft, "???");
            return;
        }

        bool isActive = (bool)get<0>(gateway->goldFlowFrom_view);
        displayWorkOrderInfo(window, font, upperLeft, incomeJobName, jobCompletion, gateway->scuttleTargetQueue.size() - 1, fundsLeftInQueue, "extraction", nameColor, isActive);
        return;
    }
    else
    {
        displayWorkOrderInfo(window, font, upperLeft, "No extraction jobs", {}, 0, {}, "extraction", sf::Color(150, 150, 150), false);
    }
}

void displayGatewayProfitStatus(sf::RenderWindow* window, sf::Font* font, vector2i upperLeft, int drawWidth, boost::shared_ptr<Gateway> gateway)
{
    string longTextString;
    bool showPlus = false;
    bool showMinus = false;
    sf::Color characterTextColor, longTextColor;

    if (!(get<0>(gateway->goldFlowFrom_view) || get<0>(gateway->goldFlowTo_view)))
    {
        // Nothing interesting happening, so we don't need to display anything
        longTextString = "Idle";
        characterTextColor = sf::Color(150, 150, 150);
        longTextColor = sf::Color(180, 180, 180);
    }
    else if (get<0>(gateway->goldFlowFrom_view) && get<0>(gateway->goldFlowTo_view))
    {
        showPlus = showMinus = true;
        longTextString = "break-even";
        characterTextColor = sf::Color(150, 150, 150);
        longTextColor = sf::Color(180, 180, 180);
    }
    else if (get<0>(gateway->goldFlowFrom_view))
    {
        showPlus = true;
        longTextString = "Sending gold to wallet.";
        characterTextColor = sf::Color(0, 255, 0);
        longTextColor = sf::Color(150, 200, 150);
    }
    else
    {
        showMinus = true;
        longTextString = "Withdrawing gold from wallet.";
        characterTextColor = sf::Color(255, 0, 0);
        longTextColor = sf::Color(200, 150, 150);
    }

    // vector2fl ckbSymbolDrawPos = upperLeft + vector2fl(drawWidth / 2, 0);
    // displayCKBSymbol(window, 20, characterTextColor, ckbSymbolDrawPos, vector2fl(0.5, 0));

    sf::Text dollarSign("$", *font, 28);
    int xOffset = (drawWidth - dollarSign.getLocalBounds().width) / 2;
    vector2i dollarSignDrawPos = upperLeft + vector2i(xOffset, 0);
    dollarSign.setPosition(toSFVecF(dollarSignDrawPos));
    dollarSign.setFillColor(characterTextColor);
    window->draw(dollarSign);

    if (showPlus)
    {
        sf::Text plus("+", *font, 28);
        plus.setPosition(toSFVecF(dollarSignDrawPos + vector2i(13, 0)));
        plus.setFillColor(characterTextColor);
        window->draw(plus);
    }
    if (showMinus)
    {
        sf::Text minus("-", *font, 28);
        minus.setPosition(toSFVecF(dollarSignDrawPos + vector2i(-13, 0)));
        minus.setFillColor(characterTextColor);
        window->draw(minus);
    }

    int yOffset = 32;
    sf::Text longText(longTextString, *font, 14);
    xOffset = (drawWidth - longText.getLocalBounds().width) / 2;
    vector2i longTextDrawPos = upperLeft + vector2i(xOffset, yOffset);
    longText.setPosition(toSFVecF(longTextDrawPos));
    longText.setFillColor(longTextColor);
    window->draw(longText);
}

void displayGatewayStatus(sf::RenderWindow* window, sf::Font* font, vector2i upperLeft, int availableWidth, boost::shared_ptr<Gateway> gateway)
{
    int yOffset = 0;

    displayGatewayProfitStatus(window, font, upperLeft + vector2i(0, yOffset), availableWidth, gateway);

    yOffset += 100;

    displayGatewayGoldSpendInfo(window, font, upperLeft + vector2i(0, yOffset), gateway);

    yOffset += 50;

    displayGatewayGoldIncomeInfo(window, font, upperLeft + vector2i(0, yOffset), gateway);
}

void displayCombatUnitStatus(sf::RenderWindow* window, sf::Font* font, vector2i upperLeft, int availableWidth, boost::shared_ptr<CombatUnit> combatUnit)
{
    string statusString;
    if (combatUnit->state == CombatUnit::State::AttackingGeneral)
    {
        if (combatUnit->maybeAttackObjective)
        {
            statusString = "Assaulting location";
        }
        else
        {
            statusString = "Engaging nearby enemy";
        }
    }
    else if (combatUnit->state == CombatUnit::State::AttackingSpecific)
    {
        statusString = "Targeting enemy";
    }
    else if (combatUnit->state == CombatUnit::State::NotAttacking)
    {
        if (auto mobileUnit = dynamic_cast<MobileUnit*>(combatUnit.get()))
        {
            if (mobileUnit->mobileUnitIsIdle())
            {
                statusString = "Idle";
            }
            else
            {
                statusString = "Moving to position";
            }
        }
    }

    sf::Text text(statusString, *font, 16);
    text.setPosition(toSFVecF(upperLeft));
    text.setFillColor(sf::Color::White);
    window->draw(text);
}

void displayFighterStatus(sf::RenderWindow* window, sf::Font* font, vector2i upperLeft, int availableWidth, boost::shared_ptr<Fighter> fighter)
{
    displayCombatUnitStatus(window, font, upperLeft, availableWidth, fighter);
}
void displayTurretStatus(sf::RenderWindow* window, sf::Font* font, vector2i upperLeft, int availableWidth, boost::shared_ptr<Turret> turret)
{
    displayCombatUnitStatus(window, font, upperLeft, availableWidth, turret);
}
void displayBeaconStatus(sf::RenderWindow* window, sf::Font* font, vector2i upperLeft, int availableWidth, boost::shared_ptr<Beacon> beacon)
{
    string statusString;
    if (beacon->state == Beacon::State::Spawning)
    {
        statusString = "Warping in (" + floatToShortPercentString(beacon->getBuiltRatio()) + ")";
    }
    else
    {
        statusString = "Warping out (" + floatToShortPercentString(1 - beacon->getBuiltRatio()) + ")";
    }
    sf::Text statusText(statusString, *font, 16);
    statusText.setPosition(toSFVecF(upperLeft));
    statusText.setFillColor(sf::Color::White);
    window->draw(statusText);
}

void displayUnitStatus(sf::RenderWindow* window, vector2i upperLeft, int availableWidth, boost::shared_ptr<Unit> unit, sf::Font* font)
{
    int xOffset = 20;
    vector2i detailedStatusDrawPos = upperLeft + vector2i(xOffset, 0);
    int detailedStatusAvailableWidth = availableWidth - xOffset;

    if (!unit->isFullyBuilt())
    {
        if (auto beacon = boost::dynamic_pointer_cast<Beacon, Unit>(unit))
        {
            displayBeaconStatus(window, font, detailedStatusDrawPos, detailedStatusAvailableWidth, beacon);
        }
        else
        {
            string s = "Partially constructed (" + floatToShortPercentString(unit->getBuiltRatio()) + ")";
            sf::Text constructingText(s, *font, 18);
            constructingText.setFillColor(sf::Color(255, 150, 150));
            constructingText.setPosition(toSFVecF(detailedStatusDrawPos));
            window->draw(constructingText);
        }
    }
    else if (!unit->isActive())
    {
        sf::Text inactiveText("Unit inactive", *font, 18);
        inactiveText.setFillColor(sf::Color(150, 150, 150));
        inactiveText.setPosition(toSFVecF(detailedStatusDrawPos));
        window->draw(inactiveText);
    }
    else if (auto prime = boost::dynamic_pointer_cast<Prime, Unit>(unit))
    {
        displayPrimeStatus(window, font, detailedStatusDrawPos, detailedStatusAvailableWidth, prime);
    }
    else if (auto gateway = boost::dynamic_pointer_cast<Gateway, Unit>(unit))
    {
        displayGatewayStatus(window, font, detailedStatusDrawPos, detailedStatusAvailableWidth, gateway);
    }
    else if (auto fighter = boost::dynamic_pointer_cast<Fighter, Unit>(unit))
    {
        displayFighterStatus(window, font, detailedStatusDrawPos, detailedStatusAvailableWidth, fighter);
    }
    else if (auto turret = boost::dynamic_pointer_cast<Turret, Unit>(unit))
    {
        displayTurretStatus(window, font, detailedStatusDrawPos, detailedStatusAvailableWidth, turret);
    }
}

void displaySingleUnitInfo(sf::RenderWindow* window, boost::shared_ptr<Unit> unit, sf::Font* font, vector2i upperLeftDrawPos, vector2i drawAreaSize)
{
    vector2i upperRightDrawPos = upperLeftDrawPos + vector2i(drawAreaSize.x, 0);

    displayUnitName(window, unit, font, upperLeftDrawPos);
    displayUnitHealth(window, unit, font, upperRightDrawPos);

    int y = upperLeftDrawPos.y + 50;

    displayUnitStatus(window, vector2i(upperLeftDrawPos.x, y), drawAreaSize.x, unit, font);
}

void displayNoneUnitInfo(sf::RenderWindow* window, sf::Font* font, vector2i upperLeft, vector2i drawAreaSize)
{

}

void displayMultipleUnitInfo(sf::RenderWindow* window, sf::Font* font, vector<boost::shared_ptr<Unit>>* selectedUnits, vector2i upperLeft, vector2i drawAreaSize, sf::Sprite* (*getSpriteForUnitTypechar)(uint8_t))
{
    vector<tuple<uint8_t, int>> typesToCount =
    {
        {GATEWAY_TYPECHAR, 0},
        {BEACON_TYPECHAR, 0},
        {PRIME_TYPECHAR, 0},
        {FIGHTER_TYPECHAR, 0},
        {TURRET_TYPECHAR, 0}
    };

    // count up units
    for (unsigned int i=0; i<selectedUnits->size(); i++)
    {
        for (unsigned int j=0; j<typesToCount.size(); j++)
        {
            if ((*selectedUnits)[i]->typechar() == get<0>(typesToCount[j]))
            {
                get<1>(typesToCount[j]) ++;
            }
        }
    }

    int yOffset = 0;
    const int rowHeight = 60;

    for (unsigned int i=0; i<typesToCount.size(); i++)
    {
        int numOfType = get<1>(typesToCount[i]);
        if (numOfType > 0)
        {
            sf::Sprite* sprite = getSpriteForUnitTypechar(get<0>(typesToCount[i]));
            sprite->setPosition(toSFVecF(upperLeft + vector2i( 0, yOffset)));

            window->draw(*sprite);

            if (numOfType > 1)
            {
                stringstream ss;
                ss << "x" << numOfType;
                sf::Text countText(ss.str(), *font, 20);
                countText.setPosition(toSFVecF(upperLeft + vector2i(70, yOffset + 20)));
                countText.setFillColor(sf::Color::White);

                window->draw(countText);
            }

            yOffset += rowHeight;
        }
    }
}

void UnitInfoUXBox::drawContent(sf::RenderWindow* window, vector2i upperLeft)
{
    if (selectedUnits->size() == 0)
    {
        displayNoneUnitInfo(window, font, upperLeft, UNITINFOBOX_SIZE - (UX_BOX_PADDING * 2));
    }
    else if (selectedUnits->size() == 1)
    {
        displaySingleUnitInfo(window, (*selectedUnits)[0], font, upperLeft, UNITINFOBOX_SIZE - (UX_BOX_PADDING * 2));
    }
    else
    {
        displayMultipleUnitInfo(window, font, selectedUnits, upperLeft, UNITINFOBOX_SIZE - (UX_BOX_PADDING * 2), getSpriteForUnitTypechar);
    }
}
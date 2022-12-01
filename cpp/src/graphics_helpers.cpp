#include <optional>
#include <vector>
#include "graphics_helpers.h"
#include "common.h"

using namespace std;

GH::DashedLineGroup::DashedLineGroup(float dashLength, float offset)
    : dashLength(dashLength), offset(offset)
{
    dashedLines.clear();
    dashLength = dashLength;
}
void GH::DashedLineGroup::pushLine(vector2fl p1, vector2fl p2, sf::Color color)
{
    dashedLines.push_back(Line(p1, p2, color));
}
void GH::DashedLineGroup::render(sf::RenderWindow* window)
{
    vector<Line> allDashes;
    for (unsigned int i=0; i<dashedLines.size(); i++)
    {
        Line dashedLine = dashedLines[i];
        vector2fl p2p = dashedLine.p2 - dashedLine.p1;
        float oneDashPortion = dashLength / p2p.getMagnitude();
        float xStart = (oneDashPortion * 2) * offset;

        bool endLoop = false;
        for (float x=xStart; x < 1; x += oneDashPortion * 2)
        {
            
            vector2fl dashStart;
            // In case of a negative offset
            if (x < 0)
            {
                if (x + oneDashPortion < 0)
                    continue;

                dashStart = dashedLine.p1;
            }
            else
            {
                dashStart = dashedLine.p1 + (p2p * x);
            }
                
            vector2fl dashEnd;
            if (x + oneDashPortion >= 1)
            {
                dashEnd = dashedLine.p2;
                endLoop = true;
            }
            else
            {
                dashEnd = dashedLine.p1 + (p2p * (x + oneDashPortion));
            }

            allDashes.push_back(Line(dashStart, dashEnd, dashedLine.color));

            if (endLoop)
                break;
        }
    }

    sf::VertexArray lines(sf::Lines, allDashes.size() * 2);
    for (unsigned int i=0; i<allDashes.size(); i++)
    {
        lines[i*2].position = toSFVec(allDashes[i].p1);
        lines[i*2].color = allDashes[i].color;
        lines[i*2 + 1].position = toSFVec(allDashes[i].p2);
        lines[i*2 + 1].color = allDashes[i].color;
    }
    window->draw(lines);
}

sf::Text GH::wrapTextBlock(string text, sf::Font* font, int fontSize, int width)
{
    vector<string> words = splitLineIntoWords(text);

    string wrappedTextBlock("");
    sf::Text renderedTextBlock(sf::String(""), *font, fontSize);
    for (unsigned i=0; i<words.size(); i++)
    {
        auto word = words[i];
        string oldWrappedTextBlock = wrappedTextBlock;

        wrappedTextBlock += (i == 0 ? "" : " ") + word;
        renderedTextBlock.setString(sf::String(wrappedTextBlock));
        if (renderedTextBlock.getLocalBounds().width > width)
        {
            wrappedTextBlock = oldWrappedTextBlock + "\n" + word;
            renderedTextBlock.setString(wrappedTextBlock);
            if (renderedTextBlock.getLocalBounds().width > width)
            {
                // indicates the word itself is too big, or something else wierd
                throw "Error text wrapping.\n";
            }
        }
    }

    // uint height = renderedTextBlock.getLocalBounds().height;

    return renderedTextBlock;
}

void GH::wrapAndRenderTextWithTransform(sf::RenderWindow* window, string text, sf::Font* font, int fontSize, sf::Color color, int width, sf::Transform* transform)
{
    auto rendered = wrapTextBlock(text, font, fontSize, width);
    rendered.setFillColor(color);

    window->draw(rendered, *transform);

    transform->translate(sf::Vector2f(0, fontSize + rendered.getLocalBounds().height));
}

uint GH::wrapAndRenderTextAtPos(sf::RenderWindow* window, string text, sf::Font* font, int fontSize, sf::Color color, int width, vector2i drawPos)
{
    auto rendered = wrapTextBlock(text, font, fontSize, width);
    uint height = rendered.getLocalBounds().height;

    rendered.setFillColor(color);
    rendered.setPosition(toSFVecF(drawPos));

    window->draw(rendered);

    return height;
}
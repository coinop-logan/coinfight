#include <SFML/Graphics.hpp>
#include "myvectors.h"

#ifndef GRAPHICS_HELPERS_H
#define GRAPHICS_HELPERS_H

using namespace std;

namespace GH {

class Line {
private:
    optional<float> _length;
public:
    vector2fl p1, p2;
    sf::Color color;
    Line(vector2fl p1, vector2fl p2, sf::Color color)
        : p1(p1), p2(p2), color(color)
        {}
    float length()
    {
        if (!_length)
        {
            _length = {(p1 - p2).getMagnitude()};
        }
        return *_length;
    }
};

class DashedLineGroup {
    vector<Line> dashedLines;
    float dashLength, offset;
public:
    DashedLineGroup(float dashLength, float offset);
    void pushLine(vector2fl from, vector2fl to, sf::Color color);
    void render(sf::RenderWindow* window);
};

sf::Text wrapTextBlock(string text, sf::Font* font, int fontSize, int width);
void wrapAndRenderTextWithTransform(sf::RenderWindow* window, string text, sf::Font* font, int fontSize, sf::Color color, int width, sf::Transform* transform);
unsigned int wrapAndRenderTextAtPos(sf::RenderWindow* window, string text, sf::Font* font, int fontSize, sf::Color color, int width, vector2i drawPos);

}

#endif // GRAPHICS_HELPERS_H

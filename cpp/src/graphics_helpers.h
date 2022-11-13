#include <SFML/Graphics.hpp>
#if defined(__APPLE__)
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#else
#include <GL/gl.h>
#include <GL/glu.h>
#endif
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
    vector<Line> _dashedLines;
    float _dashLength;
public:
    DashedLineGroup(float dashLength);
    void pushLine(vector2fl from, vector2fl to, sf::Color color);
    void render(sf::RenderWindow* window);
};

}

#endif // GRAPHICS_HELPERS_H
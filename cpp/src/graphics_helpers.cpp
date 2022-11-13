#include <optional>
#include <vector>
#include "graphics_helpers.h"
#include "common.h"

using namespace std;

GH::DashedLineGroup::DashedLineGroup(float dashLength)
{
    _dashedLines.clear();
    _dashLength = dashLength;
}
void GH::DashedLineGroup::pushLine(vector2fl p1, vector2fl p2, sf::Color color)
{
    _dashedLines.push_back(Line(p1, p2, color));
}
void GH::DashedLineGroup::render(sf::RenderWindow* window)
{
    vector<Line> allDashes;
    for (unsigned int i=0; i<_dashedLines.size(); i++)
    {
        Line dashedLine = _dashedLines[i];
        vector2fl p2p = dashedLine.p2 - dashedLine.p1;
        float oneDashPortion = _dashLength / p2p.getMagnitude();
        bool endLoop = false;
        for (float x=0; x < 1; x += oneDashPortion * 2)
        {
            vector2fl dashStart = dashedLine.p1 + (p2p * x);
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
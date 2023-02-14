#include <vector>
#include <string>
#include <sstream>
#include "common/myvectors.h"
#include "interface/common.h"
#include "common/utils.h"

#include "interface/graphics/common.h"

using namespace std;

sf::Font fwFont, humanFont;

void loadFonts()
{
    if (!fwFont.loadFromFile("Andale_Mono.ttf"))
        throw runtime_error("Can't load main font");
    if (!humanFont.loadFromFile("NotoSansCJK-Regular.ttc"))
        throw runtime_error("Can't load tutorial font");
}

const vector<string> titleTextSrcLines = 
{
    " ___    ___    _    _  _    ___    _    ___    _ _    ___ ",
    "|  _}  |   |  | |  | \\| |  | __}  | |  |  _}  | | |  |_ _|",
    "| |_   | | |  | |  |  \\ |  | __}  | |  | | |  | - |   | |  ",
    "|___}  |___|  |_|  |_|\\_|  |_|    |_|  |___|  |_|_|   |_| "
     // note the "non-rendered" double backslashes here
};

void displayTitle(sf::RenderWindow* window, int yPadding)
{
    stringstream srcTextSS;
    for (unsigned int i=0; i<titleTextSrcLines.size(); i++)
    {
        if (i!=0)
        {
            srcTextSS << "\n";
        }
        srcTextSS << titleTextSrcLines[i];
    }
    string srcText = srcTextSS.str();

    sf::Text rendered(sf::String(srcText), fwFont, 20);

    unsigned int width = rendered.getLocalBounds().width;

    vector2i center = getScreenSize(window) / 2;

    vector2i drawPos(
        (center.x - (width / 2)),
        yPadding
    );
    rendered.setPosition(toSFVecF(drawPos));

    window->draw(rendered);
}

sf::Font* getHumanFont()
{
    return &humanFont;
}
sf::Font* getFWFont()
{
    return &fwFont;
}
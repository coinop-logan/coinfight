#include "interface/common.h"
#include "common/utils.h"

vector2i getScreenSize(sf::RenderWindow* window)
{
    return fromSFVec(window->getSize());
}
vector2fl getCurrentViewSize(sf::RenderWindow* window)
{
    return fromSFVec(window->getView().getSize());
}
vector2fl getViewSize(sf::RenderWindow* window, sf::View view)
{
    sf::View currentView = window->getView();

    window->setView(view);
    vector2fl size = getCurrentViewSize(window);

    window->setView(currentView);

    return size;
}
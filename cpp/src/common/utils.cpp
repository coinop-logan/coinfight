#include "common/utils.h"

sf::Vector2f toSFVecF(vector2fl v)
{
    return sf::Vector2f(v.x, v.y);
}
sf::Vector2i toSFVecI(vector2i v)
{
    return sf::Vector2i(v.x, v.y);
}
sf::Vector2f toSFVecF(vector2i v)
{
    return sf::Vector2f(v.x, v.y);
}
vector2fl fromSFVec(sf::Vector2f v)
{
    return vector2fl(v.x, v.y);
}
vector2i fromSFVec(sf::Vector2i v)
{
    return vector2i(v.x, v.y);
}
vector2i fromSFVec(sf::Vector2u v)
{
    return vector2i(v.x, v.y);
}
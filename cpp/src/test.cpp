#include <iostream>
#include <boost/bind.hpp>
#include <boost/function.hpp>
#include <optional>
#include "graphics_helpers.h"

using namespace std;

void makeSure(bool condition) // hacky test function
{
    cout << (condition ? "PASSED" : "FAILED") << endl;
}

void makeSure(string name, bool condition) // hacky test function
{
    cout << (condition ? "PASSED: " : "FAILED: ") << name << endl;
}

using namespace std;
using vch = vector<unsigned char>;

template<typename C>
void pr(C c) {
    cout << c << endl;
}

int main()
{
    // GH::beginDashedLines(5);
    // GH::pushLine(vector2fl(0, 0), vector2fl(20, 20), sf::Color::Red);
    // GH::pushLine(vector2fl(0, 0), vector2fl(50, 50), sf::Color::Red);
    // GH::endDashedLines();
}
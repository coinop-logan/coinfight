#include <cmath>
#include <iostream>
#include <boost/bind.hpp>
#include <boost/function.hpp>
#include <optional>
#include "graphics_helpers.h"
#include "fpm/math.hpp"
#include "fpm/ios.hpp"
#include "myvectors.h"

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
    fixed32 a(20490);
    fixed32 b(-24556);
    fixed32 c = a - b;
    cout << c << endl;
}
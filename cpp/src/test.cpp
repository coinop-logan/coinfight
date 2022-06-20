#include <iostream>
#include <boost/bind.hpp>
#include <boost/function.hpp>
#include <optional>
#include "myvectors.h"
#include "fpm/fixed.hpp"
#include "fpm/ios.hpp"
#include "common.h"

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
    fixed32 a(-0.65);
    uint16_t b = 10;
    uint16_t c = static_cast<uint16_t>(a * b);

    cout << c << endl;
}
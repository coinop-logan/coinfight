#include <cmath>
#include <iostream>
#include <boost/bind.hpp>
#include <boost/function.hpp>
#include <boost/filesystem.hpp>
#include <optional>
#include <functional>
#include <boost/random.hpp>
#include "algorithm.h"
#include "common.h"

using namespace std;

void makeSure(bool condition) // hacky test function
{
    cout << (condition ? "PASSED" : "FAILED") << endl;
}

void makeSure(string name, bool condition) // hacky test function
{
    cout << (condition ? "PASSED: " : "FAILED: ") << name << endl;
}

void doThing(int a)
{
    cout << a << endl;
}

int main()
{
    cout << unixTimeToTimeLeftString(time(0) + (60 * 60 * 24 * 0.5)) << endl;

    return 0;
}
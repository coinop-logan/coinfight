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
    vch data;
    Netpack::Builder b(&data);
    b.enableDebugOutput();

    b.packInt16_t(4);
    b.packBool(true);
    b.packStringWith16bitSize("ohai!");
    b.prependWith64bitSize();

    cout << endl << endl;

    Netpack::Consumer c(data.begin());
    c.enableDebugOutput();
    c.consumeUint64_t();
    c.consumeInt16_t();
    c.consumeBool();
    c.consumeStringWith16bitSize();
}
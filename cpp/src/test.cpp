#include <iostream>
#include <optional>
#include "netpack.h"
#include "fpm/fixed.hpp"

void makeSure(bool condition) // hacky test function
{
    cout << (condition ? "PASSED" : "FAILED") << endl;
}

void makeSure(string name, bool condition) // hacky test function
{
    cout << (condition ? "PASSED: " : "FAILED: ") << name << endl;
}

using namespace std;

int main()
{
    fpm::fixed_16_16 f1(-6432.7466);
    Netpack::Builder d;
    d.packInt32_t(f1.raw_value());
    // cout << d.getHexString() << endl;
    Netpack::Consumer c(d);
    fpm::fixed_16_16 f2 = fpm::fixed_16_16::from_raw_value(c.consumeInt32_t());

    makeSure(f1==f2 && (d.getHexString() == "0xe6df40df"));
    
    return 0;
}
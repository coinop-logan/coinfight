#include <iostream>
#include <optional>
#include "netpack.h"
#include "fixed.h"

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
    numeric::fixed<32, 32> f1(-92758934.3439243214374312431423);
    Netpack::Builder d;
    d.packInt64_t(f1.data_);
    Netpack::Consumer c(d);
    numeric::fixed<32, 32> f2;
    f2.data_ = c.consumeInt64_t();
    makeSure(f1==f2 && (d.getHexString() == "0xfa789c69a7f49340"));
    
    return 0;
}
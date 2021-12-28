#include <iostream>
#include <optional>
#include <typeinfo>
#include "engine.h"
#include "config.h"
#include "myvectors.h"
#include "coins.h"

using namespace std;

void makeSure(bool condition) // hacky test function
{
    cout << (condition ? "PASSED" : "FAILED") << endl;
}

void makeSure(string name, bool condition) // hacky test function
{
    cout << (condition ? "PASSED: " : "FAILED: ") << name << endl;
}

void testCoins()
{
    Coins a, b;

    a.createMoreByFiat(11);
    b.createMoreByFiat(50);

    makeSure(a.transferUpTo(10, &b) == 10);
    makeSure(a.transferUpTo(9, &b) == 1);

    cout << endl;

    Coins c, d;
    c.createMoreByFiat(MAX_COINS - 10);
    d.createMoreByFiat(20);

    makeSure(d.transferUpTo(20, &c) == 10);
    makeSure(d.transferUpTo(20, &c) == 0);

    cout << endl;

    Coins e;
    e.createMoreByFiat(20);
    makeSure(e.tryTransfer(20, &c) == false);

    cout << endl;

    Coins low, high;
    low.createMoreByFiat(5);
    high.createMoreByFiat(MAX_COINS - 8);
    makeSure(low.tryTransfer(10, &high) == false);
    makeSure(low.transferUpTo(10, &high) == 5);
}

int main()
{
    testCoins();

    return 0;
}
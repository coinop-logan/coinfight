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
    Coins a(200), b(200);

    a.createMoreByFiat(11);
    b.createMoreByFiat(50);

    makeSure(a.transferUpTo(10, &b) == 10);
    makeSure(a.transferUpTo(9, &b) == 1);

    cout << endl;

    Coins c(500), d(500);
    c.createMoreByFiat(400);
    d.createMoreByFiat(400);

    makeSure(d.transferUpTo(200, &c) == 100);
    makeSure(d.transferUpTo(200, &c) == 0);

    cout << endl;

    Coins e(500);
    e.createMoreByFiat(20);
    makeSure(e.tryTransfer(20, &c) == false);

    cout << endl;

    Coins low(MAX_COINS), high(MAX_COINS);
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
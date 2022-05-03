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
    // fpm::fixed_16_16 f1(-6432.7466);
    // Netpack::Builder d;
    // d.packInt32_t(f1.raw_value());
    // // cout << d.getHexString() << endl;
    // Netpack::Consumer c(d);
    // fpm::fixed_16_16 f2 = fpm::fixed_16_16::from_raw_value(c.consumeInt32_t());

    // makeSure(f1==f2 && (d.getHexString() == "0xe6df40df"));


    // Netpack::Builder b;
    // b.packStringWith16bitSize(string("hi there!!!"));
    // Netpack::Consumer c(b);
    // string s2 = c.consumeStringWith16bitSize();
    // cout << s2 << endl;


    // Netpack::Builder b;
    // boost::shared_ptr<Cmd> cmd11(new WithdrawCmd(55));
    // boost::shared_ptr<Cmd> cmd21(new WithdrawCmd(5666));
    // cmd11->pack(&b);
    // cmd21->pack(&b);

    // Netpack::Consumer c(b);
    // boost::shared_ptr<Cmd> cmd12 = consumeCmd(&c);
    // boost::shared_ptr<Cmd> cmd22 = consumeCmd(&c);

    // makeSure(cmd12->getTypechar() == 4);
    // auto wCmd1 = boost::dynamic_pointer_cast<WithdrawCmd, Cmd>(cmd12);
    // makeSure((bool)wCmd1);
    // makeSure(wCmd1->amount == 55);

    // makeSure(cmd22->getTypechar() == 4);
    // auto wCmd2 = boost::dynamic_pointer_cast<WithdrawCmd, Cmd>(cmd22);
    // makeSure((bool)wCmd2);
    // makeSure(wCmd2->amount == 5666);
    
    return 0;
}
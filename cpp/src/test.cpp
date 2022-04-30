#include <iostream>
#include <optional>
#include "netpack.h"

// void makeSure(bool condition) // hacky test function
// {
//     cout << (condition ? "PASSED" : "FAILED") << endl;
// }

// void makeSure(string name, bool condition) // hacky test function
// {
//     cout << (condition ? "PASSED: " : "FAILED: ") << name << endl;
// }

using namespace std;

int main()
{
    NetballSpooler nbs;

    uint8_t a = 254;
    int16_t b = 32012;
    int16_t c = -14312;
    uint16_t d = 60422;

    nbs.pack(a);
    // nbs.pack(b);
    // nbs.pack(c);
    // nbs.pack(d);

    NetballUnspooler nbu(nbs);

    cout << (int)nbu.consumeUint8_t() << endl;
    // cout << (int)nbu.consumeInt16_t() << endl;
    // cout << (int)nbu.consumeInt16_t() << endl;
    // cout << (int)nbu.consumeUint16_t() << endl;
    
    
    return 0;
}
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

    int8_t a = 3;
    int8_t b = -77;
    int8_t c = 101;

    nbs.pack(a);
    nbs.pack(b);
    nbs.pack(c);

    NetballUnspooler nbu(nbs);

    cout << (int)nbu.consumeInt8_t() << endl;
    cout << (int)nbu.consumeInt8_t() << endl;
    cout << (int)nbu.consumeInt8_t() << endl;
    
    
    return 0;
}
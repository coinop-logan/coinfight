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

    uint32_t a = 4294967291;
    int32_t b = 2047483648;
    int64_t c = -8223372036854775808;
    uint64_t d = 18446744073709551610;

    nbs.pack(a);
    nbs.pack(b);
    nbs.pack(c);
    nbs.pack(d);

    NetballUnspooler nbu(nbs);

    cout << (long long int)nbu.consumeUint32_t() << endl;
    cout << (long long int)nbu.consumeInt32_t() << endl;
    cout << (long long int)nbu.consumeInt64_t() << endl;
    cout << (long long unsigned int)nbu.consumeUint64_t() << endl;
    
    return 0;
}
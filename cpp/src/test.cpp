#include <iostream>
#include <optional>

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
    optional<int> a = {};
    optional<int> b = {4};

    cout << (a == 4) << endl;
    
    return 0;
}
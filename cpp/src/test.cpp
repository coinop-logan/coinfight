#include <cmath>
#include <iostream>
#include <boost/bind.hpp>
#include <boost/function.hpp>
#include <boost/filesystem.hpp>
#include <optional>
#include <functional>
#include <boost/random.hpp>
#include "common.h"

using namespace std;

void makeSure(bool condition) // hacky test function
{
    cout << (condition ? "PASSED" : "FAILED") << endl;
}

void makeSure(string name, bool condition) // hacky test function
{
    cout << (condition ? "PASSED: " : "FAILED: ") << name << endl;
}

void doThing(int a)
{
    cout << a << endl;
}

int main()
{
    string filename = to_string(time(0)) + "-" + to_string(clock());
    ofstream withdrawDescriptorFile("/tmp/coinfight/" + filename);
    withdrawDescriptorFile << "hi";
    withdrawDescriptorFile.close();

    boost::filesystem::copy_file("/tmp/coinfight/" + filename, "/var/run/coinfight/events_out/withdrawals/" + filename);
    boost::filesystem::remove("/tmp/coinfight/" + filename);

    return 0;
}
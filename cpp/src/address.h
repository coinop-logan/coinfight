#include <string>

#include "netpack.h"

#ifndef ADDRESS_H
#define ADDRESS_H

using namespace std;

class Address
{
    string s;
public:
    Address(string s);
    Address(Netpack::Consumer*);
    string getString() const;
    void pack(Netpack::Builder*);
    bool operator ==(const Address &other);
    bool operator !=(const Address &other);
};

#endif // ADDRESS_H
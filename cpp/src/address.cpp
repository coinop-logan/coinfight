#include "address.h"
#include "algorithm.h"

Address::Address(string _s)
{
    s = _s;
}
Address::Address(Netpack::Consumer* from)
{
    s = from->consumeStringGivenSize(42);
}
string Address::getString() const
{
    return s;
}
void Address::pack(Netpack::Builder* to)
{
    to->packStringWithoutSize(s);
}

bool Address::operator ==(const Address &other)
{
    return (lowercaseStr(s) == lowercaseStr(other.getString()));
}
bool Address::operator !=(const Address &other)
{
    return (lowercaseStr(s) != lowercaseStr(other.getString()));
}
#include <algorithm>

#ifndef COINS_H
#define COINS_H

using namespace std;

extern const unsigned int MAX_COINS;

class Coins
{
private:
    unsigned long heldAmount;
    unsigned long deductUpTo(unsigned long);
    unsigned long addUpTo(unsigned long);
    bool tryDeduct(unsigned long);
    bool tryAdd(unsigned long);
public:
    Coins();
    unsigned long getSpaceLeft();
    bool createMoreByFiat(unsigned long);
    bool destroySomeByFiat(unsigned long);
    unsigned long transferUpTo(unsigned long, Coins*);
    bool tryTransfer(unsigned long, Coins*);
};

#endif // COINS_H
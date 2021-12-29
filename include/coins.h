#include <algorithm>

#ifndef COINS_H
#define COINS_H

using namespace std;
using vch = vector<unsigned char>;
using vchIter = vector<unsigned char>::iterator;
using coinsInt = unsigned long;

extern const coinsInt MAX_COINS;

class Coins
{
private:
    coinsInt heldAmount;
    coinsInt deductUpTo(coinsInt);
    coinsInt addUpTo(coinsInt);
    bool tryDeduct(coinsInt);
    bool tryAdd(coinsInt);
public:
    coinsInt max;
    coinsInt getInt();
    Coins(coinsInt);
    Coins(vchIter*);
    coinsInt getSpaceLeft();
    bool createMoreByFiat(coinsInt);
    bool destroySomeByFiat(coinsInt);
    coinsInt transferUpTo(coinsInt, Coins*);
    bool tryTransfer(coinsInt, Coins*);
    void pack(vch*);
    void unpackAndMoveIter(vchIter*);
};

#endif // COINS_H
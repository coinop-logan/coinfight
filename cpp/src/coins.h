#include <algorithm>
#include <SFML/Graphics.hpp>
#include "config.h"
#include "netpack.h"

#ifndef COINS_H
#define COINS_H

using namespace std;

extern const coinsInt MAX_COINS;

void packCoinsInt(Netpack::Builder* to, coinsInt amount);
coinsInt consumeCoinsInt(Netpack::Consumer* from);

coinsInt weiDepositStringToCoinsInt(string weiString);
string coinsIntToWeiDepositString(coinsInt coins);
string coinsIntToDollarString(coinsInt coins);

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
    sf::String getDollarString();
    Coins();
    Coins(coinsInt);
    Coins(Netpack::Consumer*);
    coinsInt getSpaceLeft();
    bool createMoreByFiat(coinsInt);
    bool destroySomeByFiat(coinsInt);
    coinsInt transferUpTo(coinsInt, Coins*);
    bool tryTransfer(coinsInt, Coins*);
    void pack(Netpack::Builder*);
};

#endif // COINS_H
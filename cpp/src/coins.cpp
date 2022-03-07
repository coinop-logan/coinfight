#include <string>
#include "coins.h"
#include "vchpack.h"

extern const unsigned long MAX_COINS = UINT32_MAX;

Coins::Coins(coinsInt max) :  heldAmount(0), max(max)
{
    if (max > MAX_COINS)
        throw invalid_argument("Can't set Coins max to be greater than MAX_COINS");
}

coinsInt weiDepositStringToCoinsInt(string weiString)
{
    int digitsToRemove = WEI_PER_DOLLAR_EXPONENT - CREDIT_PER_DOLLAR_EXPONENT;
    string newString(weiString);
    newString.erase(newString.length() - digitsToRemove);
    return stoi(newString);
}

unsigned long Coins::deductUpTo(unsigned long deductAmount)
{
    if (tryDeduct(deductAmount))
        return deductAmount;
    else
    {
        unsigned long deducted = heldAmount;
        heldAmount = 0;
        return deducted;
    }
}
unsigned long Coins::addUpTo(unsigned long addAmount)
{
    if (tryAdd(addAmount))
        return addAmount;
    else
    {
        unsigned long added = getSpaceLeft();
        heldAmount = max;
        return added;
    }
}
bool Coins::tryDeduct(unsigned long deductAmount)
{
    if (deductAmount > heldAmount)
        return false;
    else
    {
        heldAmount -= deductAmount;
        return true;
    }
}
bool Coins::tryAdd(unsigned long addAmount)
{
    if (addAmount > getSpaceLeft())
        return false;
    else
    {
        heldAmount += addAmount;
        return true;
    }
}

// PUBLIC

coinsInt Coins::getInt()
{
    return heldAmount;
}
// sf::String Coins::getDollarString()
// {
//     float dollars = getInt() / CREDIT_PER_DOLLAR;
//     char buf[100];
//     snprintf(buf, 100, "$%.2f", dollars);
//     return sf::String(buf);
// }
unsigned long Coins::getSpaceLeft()
{
    return max - heldAmount;
}
bool Coins::createMoreByFiat(unsigned long createAmount)
{
    return tryAdd(createAmount);
}
bool Coins::destroySomeByFiat(unsigned long destroyAmount)
{
    return tryDeduct(destroyAmount);
}
unsigned long Coins::transferUpTo(unsigned long transferAmount, Coins* to)
{
    unsigned long maxPossible = min(this->heldAmount, to->getSpaceLeft());
    unsigned long finalTransferAmount = min(maxPossible, transferAmount);
    if (this->tryDeduct(finalTransferAmount) && to->tryAdd(finalTransferAmount))
    {
        return finalTransferAmount;
    }
    else
        throw logic_error("Unexpected mathematical error during Coins::transferUpTo");
}
bool Coins::tryTransfer(unsigned long transferAmount, Coins* to)
{
    unsigned long maxPossible = min(this->heldAmount, to->getSpaceLeft());
    if (transferAmount > maxPossible)
        return false;
    else
    {
        if (this->tryDeduct(transferAmount) && to->tryAdd(transferAmount))
            return true;
        else
            throw logic_error("Unexpected mathematical error during Coins::tryTransfer");
    }
}

using vch = vector<unsigned char>;
using vchIter = vector<unsigned char>::iterator;

Coins::Coins(vchIter *iter)
{
    unpackAndMoveIter(iter);
}
void Coins::pack(vch *dest)
{
    packToVch(dest, "L", heldAmount);
}
void Coins::unpackAndMoveIter(vchIter *iter)
{
    *iter = unpackFromIter(*iter, "L", &heldAmount);
}
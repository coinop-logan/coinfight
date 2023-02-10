#include <string>
#include <SFML/Graphics.hpp>
#include <cmath>
#include <sstream>
#include "coins.h"
#include "netpack.h"

void packCoinsInt(Netpack::Builder* to, coinsInt amount)
{
    to->packUint32_t(amount);
}
coinsInt consumeCoinsInt(Netpack::Consumer* from)
{
    return from->consumeUint32_t();
}

Coins::Coins()
    : heldAmount(0), max(MAX_COINS) {}

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
string coinsIntToWeiDepositString(coinsInt amount)
{
    int digitsToAdd = WEI_PER_DOLLAR_EXPONENT - CREDIT_PER_DOLLAR_EXPONENT;
    string coinsString = to_string(amount);
    string weiString = coinsString + string(digitsToAdd, '0');
    return weiString;
}

coinsInt Coins::deductUpTo(coinsInt deductAmount)
{
    if (tryDeduct(deductAmount))
        return deductAmount;
    else
    {
        coinsInt deducted = heldAmount;
        heldAmount = 0;
        return deducted;
    }
}
coinsInt Coins::addUpTo(coinsInt addAmount)
{
    if (tryAdd(addAmount))
        return addAmount;
    else
    {
        coinsInt added = getSpaceLeft();
        heldAmount = max;
        return added;
    }
}
bool Coins::tryDeduct(coinsInt deductAmount)
{
    if (deductAmount > heldAmount)
        return false;
    else
    {
        heldAmount -= deductAmount;
        return true;
    }
}
bool Coins::tryAdd(coinsInt addAmount)
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

string coinsIntToDollarString(coinsInt amount)
{
    float dollars = amount / (pow(10, CREDIT_PER_DOLLAR_EXPONENT));
    char buf[100];
    snprintf(buf, 100, "$%.2f", dollars);
    return string(buf);
}
int getNumCentsRounded(coinsInt amount)
{
    float cents = amount / (pow(10, CREDIT_PER_DOLLAR_EXPONENT - 2));
    return int(round(cents));
}
string coinsIntToCentsRoundedString(coinsInt amount)
{
    stringstream ss;
    ss << getNumCentsRounded(amount) << "c";
    return ss.str();
}
coinsInt Coins::getInt()
{
    return heldAmount;
}
string Coins::getDollarString()
{
    return coinsIntToDollarString(getInt());
}
coinsInt Coins::getSpaceLeft()
{
    return max - heldAmount;
}
bool Coins::createMoreByFiat(coinsInt createAmount)
{
    return tryAdd(createAmount);
}
bool Coins::destroySomeByFiat(coinsInt destroyAmount)
{
    return tryDeduct(destroyAmount);
}
coinsInt Coins::transferUpTo(coinsInt transferAmount, Coins* to)
{
    coinsInt maxPossible = min(this->heldAmount, to->getSpaceLeft());
    coinsInt finalTransferAmount = min(maxPossible, transferAmount);
    if (this->tryDeduct(finalTransferAmount) && to->tryAdd(finalTransferAmount))
    {
        return finalTransferAmount;
    }
    else
        throw logic_error("Unexpected mathematical error during Coins::transferUpTo");
}
bool Coins::tryTransfer(coinsInt transferAmount, Coins* to)
{
    coinsInt maxPossible = min(this->heldAmount, to->getSpaceLeft());
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

Coins::Coins(Netpack::Consumer* from)
{
    max = consumeCoinsInt(from);
    heldAmount = consumeCoinsInt(from);
}

void Coins::pack(Netpack::Builder* to)
{
    packCoinsInt(to, max);
    packCoinsInt(to, heldAmount);
}
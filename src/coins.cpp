#include "coins.h"

extern const unsigned int MAX_COINS = UINT32_MAX;

Coins::Coins() : heldAmount(0) {}

// PRIVATE

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
        heldAmount = MAX_COINS;
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

unsigned long Coins::getSpaceLeft()
{
    return MAX_COINS - heldAmount;
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
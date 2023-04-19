pragma solidity ^0.8.0;

import "@openzeppelin/contracts/access/Ownable.sol";

contract CoinfightDepositsWithdrawalsNative is Ownable {
    // owner is the game server

    constructor() {}

    // deposit is called by user (presumably through a dapp)
    // event is emitted for the server to track
    event DepositMade(address payable forAccount, uint amount);

    function deposit(address payable forAccount) external payable
    {
        require(msg.value > 0, "You're trying to deposit a value of 0...");

        emit DepositMade(forAccount, msg.value);
    }

    event HoneypotAdded(uint amount);

    function addHoneypot() external payable
    {
        require(msg.value > 0, "You're trying to create a honeypot with a value of 0...");

        emit HoneypotAdded(msg.value);
    }

    // withdraw is called by server
    // event is emitted and amount is sent to player
    event WithdrawalMade(address payable toAccount, uint amount);
    event WithdrawalFailed(address payable toAccount, uint amount);

    function withdraw(address payable toAccount, uint amount)
        external
        onlyOwner
    {
        require (amount > 0, "You're trying to process a withdrawal value of 0...");

        bool success = toAccount.send(amount);

        if (success)
        {
            emit WithdrawalMade(toAccount, amount);
        }
        else
        {
            emit WithdrawalFailed(toAccount, amount);
        }
    }
}
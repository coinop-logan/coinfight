pragma solidity ^0.8.0;

import "@openzeppelin/contracts/access/Ownable.sol";

contract CoinfightDepositsWithdrawals is Ownable {
    // owner is the game server

    // deposit is called by user (presumably through a dapp)
    // event is emitted for the server to track, and the amount is forwarded to the server
    event DepositMade(address payable forAccount, uint amount);

    function deposit(address payable forAccount)
        payable
        external
    {
        require(msg.value > 0, "You're trying to deposit with a value of 0...");

        emit DepositMade(forAccount, msg.value);

        payable(owner()).transfer(msg.value);
    }

    // withdraw is called by server
    // event is emitted and amount is sent to player
    event WithdrawalMade(address payable toAccount, uint amount);

    function withdraw(address payable toAccount)
        payable
        external
    {
        require (msg.value > 0, "You're trying to process a withdrawal value of 0...");

        emit WithdrawalMade(toAccount, msg.value);

        toAccount.transfer(msg.value);
    }
}
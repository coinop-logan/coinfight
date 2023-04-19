pragma solidity ^0.8.0;

import "@openzeppelin/contracts/access/Ownable.sol";
import "@openzeppelin/contracts/token/ERC20/IERC20.sol";

contract CoinfightDepositsWithdrawalsERC20 is Ownable {
    // owner is the game server

    IERC20 public token;

    constructor(IERC20 _token)
    {
        token = _token;
    }

    // deposit is called by user (presumably through a dapp)
    // event is emitted for the server to track, and the amount is forwarded to the server
    event DepositMade(address payable forAccount, uint amount);

    function pullDeposit(uint amount)
        internal
    {
        require(token.transferFrom(msg.sender, address(this), amount), "Token transfer failed.");
    }

    function deposit(address payable forAccount, uint amount)
        external
    {
        require(amount > 0, "You're trying to deposit with a value of 0...");

        pullDeposit(amount);

        emit DepositMade(forAccount, amount);
    }

    event HoneypotAdded(uint amount);

    function addHoneypot(uint amount)
        external
    {
        require(amount > 0, "You're trying to create a honeypot with a value of 0...");

        pullDeposit(amount);

        emit HoneypotAdded(amount);
    }

    // withdraw is called by server
    // event is emitted and amount is sent to player
    event WithdrawalMade(address payable toAccount, uint amount);

    function withdraw(address payable toAccount, uint amount)
        external
        onlyOwner
    {
        require (amount > 0, "You're trying to process a withdrawal value of 0...");

        require(token.transfer(toAccount, amount), "Token transfer failed.");

        emit WithdrawalMade(toAccount, amount);
    }
}
// SPDX-License-Identifier: MIT
pragma solidity ^0.8.0;

contract Bank {
    // Mapping from user address to balance
    mapping(address => uint256) private balances;

    /// @notice Deposit Ether into your account
    function deposit() external payable {
        require(msg.value > 0, "Deposit must be greater than 0");
        balances[msg.sender] += msg.value;
    }

    /// @notice Withdraw Ether from your account
    /// @param amount Amount in wei to withdraw
    function withdraw(uint256 amount) external {
        uint256 userBalance = balances[msg.sender];
        require(amount > 0, "Withdrawal must be greater than 0");
        require(userBalance >= amount, "Insufficient balance");

        // Checks-effects-interactions pattern
        balances[msg.sender] = userBalance - amount;

        // Transfer Ether
        (bool success, ) = msg.sender.call{value: amount}("");
        require(success, "Transfer failed");
    }

    /// @notice Get your current balance
    /// @return balance of the sender in wei
    function showBalance() external view returns (uint256) {
        return balances[msg.sender];
    }
}
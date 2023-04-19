const CoinfightDepositsWithdrawalsNative = artifacts.require("CoinfightDepositsWithdrawalsNative");

module.exports = function (deployer) {
  deployer.deploy(CoinfightDepositsWithdrawalsNative);
};

import balance_tracker
import json

def main():
    provider = balance_tracker.WEB3_PROVIDER
    w3 = balance_tracker.Web3(provider)

    address = balance_tracker.CONTRACT_ADDRESS
    abi = json.load(open('/usr/share/coinfight_server/CoinfightDepositsWithdrawals.json','r'))['abi']
    contract = w3.eth.contract(address=address, abi=abi)

    ethAccount = balance_tracker.loadEthAccount(w3)

    balance_tracker.executePendingWithdrawals(w3, contract, ethAccount)

if __name__ == "__main__":
    main()
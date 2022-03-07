from time import sleep
from web3 import Web3
import json
from pprint import pprint
import os

neededConfirmations = 5
serverAccountingDir = "../../../cpp/coinfight-simple/bin/accounting/"

def getLastBlockProcessed():
    with open("lastblock.txt", "r") as f:
        return int(f.read())

def setLastBlockProcessed(num):
    with open("lastblock.txt", "w") as f:
        f.write(str(num))

def loadEthAccount(w3):
    w3.eth.account.enable_unaudited_hdwallet_features()
    with open("../secret.txt", "r") as f:
        loadedMnemonic = f.read()
    return w3.eth.account.from_mnemonic(loadedMnemonic)

def scanForAndRecordDeposits(w3, contract):
    lastBlockProcessed = getLastBlockProcessed()
    currentBlock = w3.eth.block_number
    (startBlock, endBlock) = (lastBlockProcessed + 1, currentBlock - neededConfirmations)

    if startBlock > endBlock:
        print("No new blocks to scan.")
        return
    
    print("scanning range", startBlock, endBlock)
        
    eventFilter = contract.events.DepositMade.createFilter(fromBlock = startBlock, toBlock = endBlock)
    try:
        newLogs = eventFilter.get_all_entries()
    except ValueError as e:
        print("uh oh, the filter broke!")
        return
    
    filename = str(startBlock) + "-" + str(endBlock) + ".json"
    recordNewDeposits(newLogs, filename)

    setLastBlockProcessed(endBlock)

def executePendingWithdrawals(w3, ethAccount):
    global nextNonce

    withdrawCmdFiles = os.listdir(serverAccountingDir + "pending_withdrawals/")
    for fname in withdrawCmdFiles:
        with open(serverAccountingDir + "pending_withdrawals/" + fname, 'r') as f:
            withdrawCmd = json.load(f)

        tx = {
            'to': withdrawCmd['for'],
            'value': int(withdrawCmd['amount']),
            'gas': 21000,
            'gasPrice': 2000000000,
            'nonce': nextNonce
        }
        signed = ethAccount.sign_transaction(tx)

        # send the tx!
        txHash = w3.eth.send_raw_transaction(signed.rawTransaction).hex()
        nextNonce += 1

        print("withdrawal", txHash, withdrawCmd['for'], withdrawCmd['amount'])

        os.remove(serverAccountingDir + "pending_withdrawals/" + fname)

def main():
    provider = Web3.HTTPProvider("https://xdai-archive.blockscout.com")
    w3 = Web3(provider)

    address = "0x94e45e32aCEF6d92ca6DC92541B95e8F62De9b84"
    abi = json.load(open('../build/contracts/CoinfightDepositsWithdrawals.json','r'))['abi']
    contract = w3.eth.contract(address=address, abi=abi)

    ethAccount = loadEthAccount(w3)
    # 0xC2da5397ba829F9C144fC46a4309e307366253F7
    print("loaded account", ethAccount.address)
    
    global nextNonce
    nextNonce = w3.eth.getTransactionCount(ethAccount.address)

    while True:
        scanForAndRecordDeposits(w3, contract)
        executePendingWithdrawals(w3, ethAccount)
        sleep(10)


def recordNewDeposits(newLogs, filename):
    if len(newLogs) == 0:
        print("no new events")
    
    else:
        print("processing", len(newLogs), "logs")

        depositRecords = []
        for log in newLogs:
            forAccount = log.args.forAccount
            amountStr = str(log.args.amount)
            depositRecords.append({'for':forAccount, 'amount':amountStr})
        
        print("writing deposit events to file for game server")
        with open(serverAccountingDir + "pending_deposits/" + filename, 'w') as f:
            json.dump(depositRecords, f)

main()
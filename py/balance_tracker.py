from time import sleep
from web3 import Web3
import json, os, requests

def loadApiKey():
    with open("web3-api-key","r") as f:
        return f.read()

apiKey = loadApiKey()
web3RequestSession = requests.Session()
web3RequestSession.headers.update({'x-api-key': apiKey})

CONTRACT_ADDRESS = "0x4b21628532624867ac62875Db99dbBE21b830626"
WEB3_PROVIDER = Web3.HTTPProvider("https://gno.getblock.io/mainnet/", session = web3RequestSession)

neededConfirmations = 1
serverAccountingDir = "accounting/"

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
        
    depositEventFilter = contract.events.DepositMade.createFilter(fromBlock = startBlock, toBlock = endBlock)
    honeypotEventFilter = contract.events.HoneypotAdded.createFilter(fromBlock = startBlock, toBlock = endBlock)
    try:
        newDepositLogs = depositEventFilter.get_all_entries()
        newHoneypotLogs = honeypotEventFilter.get_all_entries()
    except ValueError as e:
        print("uh oh, the filter broke!")
        return

    filename = str(startBlock) + "-" + str(endBlock) + ".dat"
    recordNewDeposits(newDepositLogs, newHoneypotLogs, filename)

    setLastBlockProcessed(endBlock)

def executePendingWithdrawals(w3, ethAccount):
    global nextNonce

    withdrawCmdFiles = os.listdir(serverAccountingDir + "pending_withdrawals/")
    for fname in withdrawCmdFiles:
        with open(serverAccountingDir + "pending_withdrawals/" + fname, 'r') as f:
            withdrawCmdData = f.read().split(' ')
            (address, amount) = withdrawCmdData[0], withdrawCmdData[1]

        tx = {
            'to': address,
            'value': int(amount),
            'gas': 21000,
            'gasPrice': 2000000000,
            'nonce': nextNonce
        }
        signed = ethAccount.sign_transaction(tx)

        # send the tx!
        txHash = w3.eth.send_raw_transaction(signed.rawTransaction).hex()
        nextNonce += 1

        print("withdrawal", txHash, address, amount)

        os.remove(serverAccountingDir + "pending_withdrawals/" + fname)

def main():
    provider = WEB3_PROVIDER
    w3 = Web3(provider)

    try:
        getLastBlockProcessed()
    except FileNotFoundError:
        print("No data found for last block scanned.")
        startScanBlockInput = input("At which block should we start scanning? (enter for latest block)")
        if startScanBlockInput == "":
            startScanBlock = w3.eth.block_number
        else:
            startScanBlock = int(startScanBlockInput)
        
        setLastBlockProcessed(startScanBlock)

    address = CONTRACT_ADDRESS
    abi = json.load(open('CoinfightDepositsWithdrawals.json','r'))['abi']
    contract = w3.eth.contract(address=address, abi=abi)

    ethAccount = loadEthAccount(w3)
    print("loaded account", ethAccount.address)
    # 0xC2da5397ba829F9C144fC46a4309e307366253F7
    
    global nextNonce
    nextNonce = w3.eth.getTransactionCount(ethAccount.address)

    while True:
        scanForAndRecordDeposits(w3, contract)
        executePendingWithdrawals(w3, ethAccount)
        sleep(10)


def recordNewDeposits(newDepositLogs, newHoneypotLogs, filename):
    if len(newDepositLogs + newHoneypotLogs) == 0:
        print("no new events")
    
    else:
        print("processing", len(newDepositLogs), "deposits and", len(newHoneypotLogs), "new honeypot adds")

        fileLines = []
        for log in newDepositLogs:
            forAccount = log.args.forAccount
            amountStr = str(log.args.amount)
            fileLines.append(forAccount + " " + amountStr)
        
        for log in newHoneypotLogs:
            amountStr = str(log.args.amount)
            fileLines.append("honeypot " + amountStr)
            
        print("writing events to file for game server")
        with open(filename, 'w') as f:
            f.write('\n'.join(fileLines))
        
        os.system("mv " + filename + " " + serverAccountingDir + "pending_deposits/")

main()
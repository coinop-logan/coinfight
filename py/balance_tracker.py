from time import sleep
from web3 import Web3
import json, os, requests
from pprint import pprint
from hacky_event_fetch import hackyEventFetch

# def loadApiKey():
#     with open("web3-api-key","r") as f:
#         return f.read()

# apiKey = loadApiKey()
web3RequestSession = requests.Session()
# web3RequestSession.headers.update({'x-api-key': apiKey})

godwokenTestnetRpcUrl = "https://v1.testnet.godwoken.io/rpc"
godwokenRpcUrl = "https://v1.mainnet.godwoken.io/rpc"

CONTRACT_ADDRESS = "0xAa2bdef57638bF6c7d94Ed78455B84A4212b20BC"

rpcUrl = godwokenTestnetRpcUrl

WEB3_PROVIDER = Web3.HTTPProvider(rpcUrl, session = web3RequestSession)

neededConfirmations = 1
eventsToServerDir = "events_in/"
eventsFromServerDir = "events_out/"

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

def scanForAndRecordDeposits(w3, contractAddress, contractAbi):
    lastBlockProcessed = getLastBlockProcessed()
    currentBlock = w3.eth.block_number
    (startBlock, endBlock) = (lastBlockProcessed + 1, currentBlock - neededConfirmations)

    if startBlock > endBlock:
        print("No new blocks to scan.")
        return
    
    print("scanning range", startBlock, endBlock)
        
    # We want to try to "consume" each type of event one at a time,
    # so that if a filter breaks, we avoid the chance of having "consumed" one filter but broken the other
    # which may result in skipping some events on the next pass
    try:
        filename = str(startBlock) + "-" + str(endBlock) + "deposits.dat"

        # depositEventFilter = contract.events.DepositMade.createFilter(fromBlock = startBlock, toBlock = endBlock)
        # newDepositLogs = depositEventFilter.get_all_entries()
        newDepositEvents = hackyEventFetch(rpcUrl, contractAbi, contractAddress, "DepositMade", startBlock, endBlock)

        recordNewPlayerDeposits(newDepositEvents, filename)
    except ValueError as e:
        print("Exception when trying to use the player deposit event filter:", e)
        return

    try:
        filename = str(startBlock) + "-" + str(endBlock) + "honeypot.dat"
        
        # honeypotEventFilter = contract.events.HoneypotAdded.createFilter(fromBlock = startBlock, toBlock = endBlock)
        # newHoneypotLogs = honeypotEventFilter.get_all_entries()
        newHoneypotEvents = hackyEventFetch(rpcUrl, contractAbi, contractAddress, "HoneypotAdded", startBlock, endBlock)

        recordNewHoneypotDeposits(newHoneypotEvents, filename)
    except ValueError as e:
        print("Exception when trying to use the honeypot deposit event filter:", e)
        return

    setLastBlockProcessed(endBlock)

def executePendingWithdrawals(w3, contract, ethAccount):
    global nextNonce

    withdrawsDir = eventsFromServerDir + "withdrawals/"
    withdrawCmdFiles = os.listdir(withdrawsDir)
    for fname in withdrawCmdFiles:
        with open(withdrawsDir + fname, 'r') as f:
            withdrawCmdData = f.read().split(' ')
            (address, amount) = withdrawCmdData[0], int(withdrawCmdData[1])
        
        tx = contract.functions.withdraw("0x3420AA70b2f23c9a299F92EBd05fbD6c4aA21164", amount).build_transaction({
            'gas': 6000000,
            'gasPrice': 48828125000,
            'nonce': nextNonce
        })
        signed = ethAccount.signTransaction(tx)
        txHash = w3.eth.send_raw_transaction(signed.rawTransaction).hex()

        print("withdrawal", txHash, address, amount)

        os.remove(withdrawsDir + fname)

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
        
        setLastBlockProcessed(startScanBlock - 1)

    address = CONTRACT_ADDRESS
    abi = json.load(open('CoinfightDepositsWithdrawals.json','r'))['abi']
    contract = w3.eth.contract(address=address, abi=abi)

    ethAccount = loadEthAccount(w3)
    print("loaded account", ethAccount.address)
    # 0xC2da5397ba829F9C144fC46a4309e307366253F7
    
    global nextNonce
    nextNonce = w3.eth.getTransactionCount(ethAccount.address)

    while True:
        try:
            scanForAndRecordDeposits(w3, address, abi)
        except Exception as e:
            print("exception when scanning for deposits:", e)
        try:
            executePendingWithdrawals(w3, contract, ethAccount)
        except Exception as e:
            print("exception when executing pending withdrawals:", e)
            
        sleep(10)

def recordNewPlayerDeposits(newDepositEvents, filename):
    if len(newDepositEvents) == 0:
        print("no new Deposit events")
    
    else:
        print("processing", len(newDepositEvents), "deposits")

        fileLines = []
        for event in newDepositEvents:
            forAccount = event['forAccount']
            amountStr = str(event['amount'])
            fileLines.append(forAccount + " " + amountStr)
            
        print("writing player deposit events to file for game server")
        with open(filename, 'w') as f:
            f.write('\n'.join(fileLines))
        
        os.system("mv " + filename + " " + eventsToServerDir + "deposits/")

def recordNewHoneypotDeposits(newHoneypotEvents, filename):
    if len(newHoneypotEvents) == 0:
        print("no new Honeypot events")
    
    else:
        print("processing", len(newHoneypotEvents), "new honeypot adds")

        fileLines = []
        for event in newHoneypotEvents:
            amountStr = str(event['amount'])
            fileLines.append("honeypot " + amountStr)
            
        print("writing honeypot deposit events to file for game server")
        with open(filename, 'w') as f:
            f.write('\n'.join(fileLines))
        
        os.system("mv " + filename + " " + eventsToServerDir + "deposits/")

if __name__ == "__main__":
    main()
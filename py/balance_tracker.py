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

GODWOKEN_MAINNET_RPC_URL = "https://v1.mainnet.godwoken.io/rpc"
GODWOKEN_TESTNET_RPC_URL = "https://v1.testnet.godwoken.io/rpc"

GODWOKEN_TESTNET_CONTRACT_ADDRESS = "0x0c6b71d194974ae7B110f5A78ef0164445239b2E"
GODWOKEN_MAINNET_CONTRACT_ADDRESS = "0xfC75cB71AF2C67be288F07888ae6009236F59b78"

RPC_URL = GODWOKEN_MAINNET_RPC_URL
CONTRACT_ADDRESS = GODWOKEN_MAINNET_CONTRACT_ADDRESS

WEB3_PROVIDER = Web3.HTTPProvider(RPC_URL, session = web3RequestSession)

neededConfirmations = 1
COINFIGHT_RUN_DIR = "/var/run/coinfight/"
COINFIGHT_DATA_DIR = "/usr/share/coinfight_server/"
EVENTS_TO_SERVER_DIR = COINFIGHT_RUN_DIR + "events_in/"
EVENTS_FROM_SERVER_DIR = COINFIGHT_RUN_DIR + "events_out/"

def getLastBlockProcessed():
    with open(COINFIGHT_DATA_DIR + "lastblock.txt", "r") as f:
        return int(f.read())

def setLastBlockProcessed(num):
    with open(COINFIGHT_DATA_DIR + "lastblock.txt", "w") as f:
        f.write(str(num))

def loadEthAccount(w3):
    w3.eth.account.enable_unaudited_hdwallet_features()
    with open(COINFIGHT_DATA_DIR + "web3_secret", "r") as f:
        loadedMnemonic = f.read()
    return w3.eth.account.from_mnemonic(loadedMnemonic)

def scanForAndRecordDeposits(w3, contractAddress, contractAbi):
    lastBlockProcessed = getLastBlockProcessed()
    currentBlock = w3.eth.block_number
    (startBlock, endBlock) = (lastBlockProcessed + 1, currentBlock - neededConfirmations)

    if startBlock > endBlock:
        # print("No new blocks to scan.")
        return
    
    # print("scanning range", startBlock, endBlock)
        
    # We want to try to "consume" each type of event one at a time,
    # so that if a filter breaks, we avoid the chance of having "consumed" one filter but broken the other
    # which may result in skipping some events on the next pass
    try:
        filename = str(startBlock) + "-" + str(endBlock) + "deposits.dat"

        # depositEventFilter = contract.events.DepositMade.createFilter(fromBlock = startBlock, toBlock = endBlock)
        # newDepositLogs = depositEventFilter.get_all_entries()
        newDepositEvents = hackyEventFetch(RPC_URL, contractAbi, contractAddress, "DepositMade", startBlock, endBlock)

        recordNewPlayerDeposits(newDepositEvents, filename)
    except ValueError as e:
        print("Exception when trying to use the player deposit event filter:", e)
        return

    try:
        filename = str(startBlock) + "-" + str(endBlock) + "honeypot.dat"
        
        # honeypotEventFilter = contract.events.HoneypotAdded.createFilter(fromBlock = startBlock, toBlock = endBlock)
        # newHoneypotLogs = honeypotEventFilter.get_all_entries()
        newHoneypotEvents = hackyEventFetch(RPC_URL, contractAbi, contractAddress, "HoneypotAdded", startBlock, endBlock)

        recordNewHoneypotDeposits(newHoneypotEvents, filename)
    except ValueError as e:
        print("Exception when trying to use the honeypot deposit event filter:", e)
        return

    setLastBlockProcessed(endBlock)

def executePendingWithdrawals(w3, contract, ethAccount):
    withdrawsDir = EVENTS_FROM_SERVER_DIR + "withdrawals/"
    withdrawCmdFiles = os.listdir(withdrawsDir)
    for fname in withdrawCmdFiles:
        with open(withdrawsDir + fname, 'r') as f:
            withdrawCmdData = f.read().split(' ')
            (address, amount) = w3.toChecksumAddress(withdrawCmdData[0]), int(withdrawCmdData[1])
        
        tx = contract.functions.withdraw(address, amount).build_transaction({
            'gas': 6000000,
            'gasPrice': 48828125000,
            'nonce': w3.eth.getTransactionCount(ethAccount.address)
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
        startScanBlock = w3.eth.block_number
        setLastBlockProcessed(startScanBlock - 1)

    address = CONTRACT_ADDRESS
    abi = json.load(open('/usr/share/coinfight_server/CoinfightDepositsWithdrawals.json','r'))['abi']
    contract = w3.eth.contract(address=address, abi=abi)

    ethAccount = loadEthAccount(w3)
    print("loaded account", ethAccount.address)
    # 0x3C5adE7Af2f9D187393F27DC2841EAFa13ab1042

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
        pass
        # print("no new Deposit events")
    
    else:
        print("processing", len(newDepositEvents), "deposits")

        fileLines = []
        for event in newDepositEvents:
            forAccount = event['forAccount']
            amountStr = str(event['amount'])
            fileLines.append(forAccount + " " + amountStr)
            
        # print("writing player deposit events to file for game server")
        with open("/tmp/coinfight/" + filename, 'w') as f:
            f.write('\n'.join(fileLines))
        
        os.system("mv /tmp/coinfight/" + filename + " " + EVENTS_TO_SERVER_DIR + "deposits/")

def recordNewHoneypotDeposits(newHoneypotEvents, filename):
    if len(newHoneypotEvents) == 0:
        pass
        # print("no new Honeypot events")
    
    else:
        print("processing", len(newHoneypotEvents), "new honeypot adds")

        fileLines = []
        for event in newHoneypotEvents:
            amountStr = str(event['amount'])
            fileLines.append("honeypot " + amountStr)
            
        # print("writing honeypot deposit events to file for game server")
        with open("/tmp/coinfight/" + filename, 'w') as f:
            f.write('\n'.join(fileLines))
        
        os.system("mv /tmp/coinfight/" + filename + " " + EVENTS_TO_SERVER_DIR + "deposits/")

if __name__ == "__main__":
    main()
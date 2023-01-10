from web3 import Web3
import json, requests
from pprint import pprint
import eth_event

def hackyEventFetch(rpcUrl, abi, contractAddress, eventName, fromBlock, toBlock):
    topicMap = eth_event.get_topic_map(abi)

    foundNamedEvent = False
    for topicSig, topic in topicMap.items():
        if topic['name'] == eventName:
            eventSig = topicSig
            foundNamedEvent = True
            break
    if not foundNamedEvent:
        raise ValueError

    session = requests.Session()

    params = {"jsonrpc": "2.0","method": "eth_getLogs","params": [{"fromBlock": hex(fromBlock),"toBlock": hex(toBlock),"topics": [eventSig],"address": contractAddress}],"id":0}
    response = session.post(rpcUrl, json = params)
    jsonData = response.json()
    decodedLogs = eth_event.decode_logs(jsonData['result'], topicMap)
    
    eventsToReturn = []
    for decodedLog in decodedLogs:
        event = {}
        for dataItem in decodedLog['data']:
            event[dataItem['name']] = dataItem['value']
        
        eventsToReturn.append(event)
    
    return eventsToReturn
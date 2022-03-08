from web3 import Web3
from hexbytes import HexBytes
from eth_account.messages import encode_defunct

def signed_msg_to_address(msgStr, sigStr):
    w3 = Web3()

    encodedMsg = encode_defunct(text=msgStr)

    encodedSig = HexBytes(sigStr)

    return w3.eth.account.recover_message(encodedMsg, signature=encodedSig)
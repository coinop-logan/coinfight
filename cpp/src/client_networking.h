#include <iostream>
#include <vector>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include "common.h"
#include "events.h"
#include "packets.h"

#ifndef CLIENT_NETWORKING_H
#define CLIENT_NETWORKING_H

using namespace std;
using namespace boost::asio::ip;

using vch = vector<unsigned char>;

class ConnectionHandler
{
    vch receivedBytes;
    boost::asio::io_service &ioService;
    tcp::socket &socket;

    vector<vch *> packetsToSend;
    bool sending;

public:
    vector<FrameEventsPacket> receivedFrameEventsPackets;
    vector<Game> receivedResyncs;

    ConnectionHandler(boost::asio::io_service &ioService, tcp::socket &socket);
    string receiveSigChallenge();
    void sendSignature(string sig);
    Address receiveAddress();
    void startReceivingLoop();

    void clearVchAndReceiveNextPacket();
    void typecharAndSizeReceived(const boost::system::error_code &error, size_t received);
    void clearVchAndReceiveResyncPacket(uint64_t size);
    void clearVchAndReceiveFrameCmdsPacket(uint64_t size);
    void resyncPacketReceived(const boost::system::error_code &error, size_t received);
    void frameCmdsPacketReceived(const boost::system::error_code &error, size_t received);
    void sendCmd(boost::shared_ptr<Cmd> cmd);
    void sendNextPacketIfNotBusy();
    void wrapUpSendingPacket(vch *sourceToDispose, const boost::system::error_code &error, size_t bytes_transferred);
};

#endif // CLIENT_NETWORKING_H
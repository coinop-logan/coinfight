#include "client_networking.h"

ConnectionHandler::ConnectionHandler(boost::asio::io_service &ioService, tcp::socket &socket)
    : ioService(ioService), socket(socket)
{
    sending = false;
}
string ConnectionHandler::receiveSigChallenge()
{
    boost::asio::streambuf buf(50);
    boost::asio::read(socket, buf);
    return string(boost::asio::buffer_cast<const char*>(buf.data()), buf.size());
}
void ConnectionHandler::sendSignature(string sig)
{
    boost::asio::write(socket, boost::asio::buffer(sig));
}
optional<Address> ConnectionHandler::receiveAddressIfNotDenied()
{
    boost::asio::streambuf successBuf(1);
    boost::asio::read(socket, successBuf);
    if (string(boost::asio::buffer_cast<const char*>(successBuf.data()), successBuf.size()) == "0")
    {
        return {};
    }
    else
    {
        boost::asio::streambuf buf(42);
        boost::asio::read(socket, buf);
        return Address(string(boost::asio::buffer_cast<const char*>(buf.data()), buf.size()));
    }
}
void ConnectionHandler::startReceivingLoop()
{
    clearVchAndReceiveNextPacket();
}

void ConnectionHandler::clearVchAndReceiveNextPacket()
{
    receivedBytes = vch(9);

    async_read(socket,
                boost::asio::buffer(receivedBytes),
                boost::bind(&ConnectionHandler::typecharAndSizeReceived,
                            this,
                            boost::asio::placeholders::error,
                            boost::asio::placeholders::bytes_transferred));
}
void ConnectionHandler::typecharAndSizeReceived(const boost::system::error_code &error, size_t received)
{
    if (!error)
    {
        Netpack::Consumer typecharAndSizeData(receivedBytes.begin());

        uint64_t size = typecharAndSizeData.consumeUint64_t();

        uint8_t packetTypechar = typecharAndSizeData.consumeUint8_t();
        size -= 1; // since that included the packetTypechar we also just consumed

        switch (packetTypechar)
        {
        case PACKET_RESYNC_CHAR:
            clearVchAndReceiveResyncPacket(size);
            break;

        case PACKET_FRAMECMDS_CHAR:
            clearVchAndReceiveFrameCmdsPacket(size);
            break;
        }
    }
    else
    {
        cout << error.message() << endl;
        throw runtime_error("Error when receiving typechar");
    }
}
void ConnectionHandler::clearVchAndReceiveResyncPacket(uint64_t size)
{
    receivedBytes = vch(size);

    async_read(socket,
                boost::asio::buffer(receivedBytes),
                boost::bind(&ConnectionHandler::resyncPacketReceived,
                            this,
                            boost::asio::placeholders::error,
                            boost::asio::placeholders::bytes_transferred));
}
void ConnectionHandler::clearVchAndReceiveFrameCmdsPacket(uint64_t size)
{
    receivedBytes = vch(size);

    async_read(socket,
                boost::asio::buffer(receivedBytes),
                boost::bind(&ConnectionHandler::frameCmdsPacketReceived,
                            this,
                            boost::asio::placeholders::error,
                            boost::asio::placeholders::bytes_transferred));
}
void ConnectionHandler::resyncPacketReceived(const boost::system::error_code &error, size_t received)
{
    if (!error)
    {
        Netpack::Consumer data(receivedBytes.begin());

        receivedResyncs.push_back(Game(&data));
        
        clearVchAndReceiveNextPacket();
    }
    else
    {
        throw runtime_error("Error when receiving resync packet");
    }
}
void ConnectionHandler::frameCmdsPacketReceived(const boost::system::error_code &error, size_t received)
{
    if (error)
    {
        throw runtime_error("Error receiving frameCmds packet:" + error.value());
    }
    else
    {
        Netpack::Consumer data(receivedBytes.begin());

        receivedFrameEventsPackets.push_back(FrameEventsPacket(&data));

        clearVchAndReceiveNextPacket();
    }
}

void ConnectionHandler::sendCmd(boost::shared_ptr<Cmd> cmd)
{
    packetsToSend.push_back(new vch);

    clearVchAndBuildCmdPacket(packetsToSend.back(), cmd);

    sendNextPacketIfNotBusy();
}
void ConnectionHandler::sendNextPacketIfNotBusy()
{
    if (!sending && packetsToSend.size() > 0)
    {
        sending = true;

        boost::asio::async_write(socket,
                                    boost::asio::buffer(*(packetsToSend[0])),
                                    boost::bind(&ConnectionHandler::wrapUpSendingPacket,
                                                this,
                                                packetsToSend[0],
                                                boost::asio::placeholders::error,
                                                boost::asio::placeholders::bytes_transferred));
    }
}
void ConnectionHandler::wrapUpSendingPacket(vch *sourceToDispose, const boost::system::error_code &error, size_t bytes_transferred)
{
    if (error)
    {
        throw runtime_error("ConnectionHandler error sending packet:" + error.value());
    }
    else
    {
        assert(packetsToSend[0] == sourceToDispose);

        delete packetsToSend[0];
        packetsToSend.erase(packetsToSend.begin());

        sending = false;

        sendNextPacketIfNotBusy();
    }
}
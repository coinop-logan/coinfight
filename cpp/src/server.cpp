#include <iostream>
#include <boost/shared_ptr.hpp>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/filesystem.hpp>
#include <boost/algorithm/string.hpp>
#include <filesystem>
#include <vector>
#include <string>
#include "cmds.h"
#include "engine.h"
#include "config.h"
#include "packets.h"
#include "sigWrapper.h"
#include "events.h"

const bool SEND_REGULAR_RESYNC_CHECKS = true;

using namespace std;
using namespace boost::asio::ip;

using vch = vector<unsigned char>;

class ClientChannel;

vector<ClientChannel *> clientChannels;

class Listener
{
    boost::asio::io_service &ioService;
    tcp::acceptor acceptor;

public:
    Listener(boost::asio::io_service &ioService_)
        : ioService(ioService_), acceptor(ioService_, tcp::endpoint(tcp::v4(), MAIN_PORT))
    {
    }
    void startAccept()
    {
        boost::shared_ptr<tcp::socket> socket(new tcp::socket(ioService));

        acceptor.async_accept(*socket, boost::bind(&Listener::handleAccept, this, socket, boost::asio::placeholders::error));
    }
    void handleAccept(boost::shared_ptr<tcp::socket> socket, const boost::system::error_code &error);
};

Game game;

void testHandler(const boost::system::error_code &error, size_t numSent)
{
    if (!error)
    {
        cout << "woo! " << numSent << endl;
    }
    else
    {
        cout << "boooooo" << endl;
    }
}

vector<boost::shared_ptr<AuthdCmd>> pendingCmds;

class ClientChannel
{
    boost::asio::io_service &ioService;
    boost::shared_ptr<tcp::socket> socket;

    vector<vch *> packetsToSend;
    bool sending;

    vch receivedBytes;
    boost::asio::streambuf receivedSig;

    string sentChallenge;

    string genRandomString(int len)
    {
        // hacky, untested, probably insecure!! Only good for the hackathon and a demo.
        static const char alphanum[] =
            "0123456789"
            "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
            "abcdefghijklmnopqrstuvwxyz";
        std::string s;
        s.reserve(len);

        for (int i = 0; i < len; ++i) {
            int randChoice = (int)((double)rand() / ((double)RAND_MAX + 1) * (sizeof(alphanum) - 1));
            s += alphanum[randChoice];
        }

        return s;
    }

public:
    enum State {
        DoingHandshake,
        ReadyForFirstSync,
        UpToDate,
        Closed
    } state;
    Address connectionAuthdUserAddress;
    ClientChannel(boost::asio::io_service &ioService_, boost::shared_ptr<tcp::socket> socket_)
        : ioService(ioService_), socket(socket_), receivedSig(150), connectionAuthdUserAddress(zeroAddress)
    {
        state = DoingHandshake;
        sending = false;
    }

    void startHandshakeAsync()
    {
        generateAndSendSigChallenge();
        receiveSigAsync(); // also kicks off receiving loop
    }

    void generateAndSendSigChallenge()
    {
        string challenge = genRandomString(50);

        boost::asio::write(*socket, boost::asio::buffer(challenge));

        cout << "sent challenge" << endl;

        sentChallenge = challenge;
    }

    void receiveSigAsync()
    {
        boost::asio::async_read_until(*socket,
                   receivedSig,
                   '\n',
                   boost::bind(&ClientChannel::sigReceived,
                               this,
                               boost::asio::placeholders::error,
                               boost::asio::placeholders::bytes_transferred));
    }

    void sigReceived(const boost::system::error_code &error, size_t transferred)
    {
        cout << "sig received" << endl;
        if (error)
        {
            cout << "Error receiving sig from " << connectionAuthdUserAddress.getString() << ". Kicking." << endl;
            state = Closed;
        }
        else
        {
            // receivedSig now has sig
            // But leave out the trailing \n leftover
            string sig(boost::asio::buffer_cast<const char*>(receivedSig.data()), receivedSig.size() - 1);

            if (sig == string("fake1"))
            {
                connectionAuthdUserAddress = string("0x798D9726775BD490e2456127e64440bdbbc54abB");
            }
            else if (sig == string("fake2"))
            {
                connectionAuthdUserAddress = string("0xFd00CF60a6a06cd101177062C690f963C2DBfFB2");
            }
            else if (sig == string("fake3"))
            {
                connectionAuthdUserAddress = string("0x93d42e141D1B79D61Ec61b21261fd8B872d284d6");
            }
            else
            {
                // now have sig and sentChallenge as strings.
                string error;
                if (auto maybeRecoveredAddress = signedMsgToAddressString(sentChallenge, sig, &error))
                {
                    connectionAuthdUserAddress = *maybeRecoveredAddress;
                }
                else
                {
                    // send a 0 to indicate failure to client
                    boost::asio::write(*socket, boost::asio::buffer("0"));
                    cout << "Error recovering address from connection. Kicking." << endl << "Here's the Python error message:" << endl;
                    cout << error << endl;
                    state = Closed;
                    return;
                }
            }

            cout << "Player authenticated and connected." << endl;

            // should really return a fail/success code here. On fail client just hangs atm.
            string response = "1" + connectionAuthdUserAddress.getString(); // prepend with 1 to indicate success
            boost::asio::write(*socket, boost::asio::buffer(response));

            state = ReadyForFirstSync;
            startReceivingLoop();
        }
    }

    void sendResyncPacket()
    {
        packetsToSend.push_back(new vch);

        clearVchAndBuildResyncPacket(packetsToSend.back(), &game);

        sendNextPacketIfNotBusy();
    }

    void sendFrameCmdsPacket(FrameEventsPacket fep)
    {
        packetsToSend.push_back(new vch);

        clearVchAndBuildFrameCmdsPacket(packetsToSend.back(), fep);

        sendNextPacketIfNotBusy();
    }

    void startReceivingLoop()
    {
        clearVchAndReceiveNextCmd();
    }

    void sendNextPacketIfNotBusy()
    {
        if (!sending && packetsToSend.size() > 0)
        {
            sending = true;
            boost::asio::async_write(*socket,
                                     boost::asio::buffer(*(packetsToSend[0])),
                                     boost::bind(&ClientChannel::wrapUpSendingPacket,
                                                 this,
                                                 packetsToSend[0],
                                                 boost::asio::placeholders::error,
                                                 boost::asio::placeholders::bytes_transferred));
        }
    }

    void wrapUpSendingPacket(vch *sourceToDispose, const boost::system::error_code &error, size_t bytes_transferred)
    {
        if (error)
        {
            cout << "Error sending packet to " << connectionAuthdUserAddress.getString() << ": " << error.message() << endl << "Kicking." << endl;
            state = Closed;
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

    void clearVchAndReceiveNextCmd()
    {
        receivedBytes = vch(2);

        async_read(*socket,
                   boost::asio::buffer(receivedBytes),
                   boost::bind(&ClientChannel::sizeReceived,
                               this,
                               boost::asio::placeholders::error,
                               boost::asio::placeholders::bytes_transferred));
    }
    void sizeReceived(const boost::system::error_code &error, size_t transferred)
    {
        if (error)
        {
            cout << "Error receiving cmd size from " << connectionAuthdUserAddress.getString() << ": " << error.message() << endl << "Kicking." << endl;
            state = Closed;
        }
        else
        {
            Netpack::Consumer source(receivedBytes.begin());

            uint16_t size = source.consumeUint16_t();

            clearVchAndReceiveCmdBody(size);
        }
    }
    void clearVchAndReceiveCmdBody(uint16_t size)
    {
        receivedBytes = vch(size);

        async_read(*socket,
                   boost::asio::buffer(receivedBytes),
                   boost::bind(&ClientChannel::cmdBodyReceived,
                               this,
                               boost::asio::placeholders::error,
                               boost::asio::placeholders::bytes_transferred));
    }
    void cmdBodyReceived(const boost::system::error_code &error, size_t transferred)
    {
        if (error)
        {
            cout << "Error receiving cmd body from " << connectionAuthdUserAddress.getString() << ": " << error.message() << endl;
            cout << "Kicking." << endl;
            state = Closed;
        }
        else
        {
            Netpack::Consumer source(receivedBytes.begin());

            boost::shared_ptr<Cmd> cmd = consumeCmd(&source);
            boost::shared_ptr<AuthdCmd> authdCmd = boost::shared_ptr<AuthdCmd>(new AuthdCmd(cmd, this->connectionAuthdUserAddress));

            pendingCmds.push_back(authdCmd);

            clearVchAndReceiveNextCmd();
        }
    }
};

void Listener::handleAccept(boost::shared_ptr<tcp::socket> socket, const boost::system::error_code &error)
{
    if (error)
    {
        throw("Listener error accepting:" + error.value());
    }
    else
    {
        cout << "client connected!" << endl;
        ClientChannel *clientChannel = new ClientChannel(ioService, socket);
        clientChannel->startHandshakeAsync();

        clientChannels.push_back(clientChannel);

    }
    startAccept();
}

struct WithdrawEvent
{
    Address userAddress;
    coinsInt amountInCoins;
    WithdrawEvent(Address userAddress, coinsInt amountInCoins)
        : userAddress(userAddress), amountInCoins(amountInCoins) {}
    boost::shared_ptr<Event> toEventSharedPtr()
    {
        return boost::shared_ptr<Event>(new BalanceUpdateEvent(userAddress, amountInCoins, false));
    }
};

void actuateWithdrawal(Address userAddress, coinsInt amount)
{
    string weiString = coinsIntToWeiDepositString(amount);
    string writeData = userAddress.getString() + " " + weiString;

    string filename = to_string(time(0)) + "-" + to_string(clock());
    ofstream withdrawDescriptorFile(filename);
    withdrawDescriptorFile << writeData;
    withdrawDescriptorFile.close();

    filesystem::rename(filename, "./events_out/withdrawals/" + filename);
}

vector<boost::shared_ptr<Event>> pollPendingEvents()
{
    vector<boost::shared_ptr<Event>> events;

    boost::filesystem::path depositsDirPath("./events_in/deposits/");
    boost::filesystem::directory_iterator directoryEndIter; // default constructor makes it an end_iter

    for (boost::filesystem::directory_iterator dirIter(depositsDirPath); dirIter != directoryEndIter; dirIter++)
    {
        if (boost::filesystem::is_regular_file(dirIter->path())) {
            string depositFilePath = dirIter->path().string();
            std::ifstream depositFile(depositFilePath);
            string depositData;

            if (depositFile.is_open())
            {
                while (depositFile)
                {
                    string depositCmdData;
                    getline(depositFile, depositCmdData);
                    if (depositCmdData.length() == 0)
                        continue;

                    vector<string> splitted;
                    boost::split(splitted, depositCmdData, boost::is_any_of(" "));
                    string userAddressOrHoneypotString = splitted[0];
                    string depositWeiString = splitted[1];
                    coinsInt depositInCoins = weiDepositStringToCoinsInt(depositWeiString);

                    if (userAddressOrHoneypotString == "honeypot")
                    {
                        events.push_back(boost::shared_ptr<Event>(new HoneypotAddedEvent(depositInCoins)));
                    }
                    else
                    {
                        events.push_back(boost::shared_ptr<Event>(new BalanceUpdateEvent(Address(userAddressOrHoneypotString), depositInCoins, true)));
                    }
                }
            }
            else
            {
                throw runtime_error("Couldn't open a deposit file...\n");
            }
            depositFile.close();
            // delete the file, having processed it
            boost::filesystem::remove(dirIter->path());
        }
    }

    boost::filesystem::path eventsDirPath("./events_in/");
    directoryEndIter = boost::filesystem::directory_iterator(); // default constructor makes it an end_iter

    bool resetBeacons = false;
    for (boost::filesystem::directory_iterator dirIter(eventsDirPath); dirIter != directoryEndIter; dirIter++)
    {
        // right now just consumes ANY REGULAR FILE in this dir as a beacon reset
        // once more events are at play, we should check the name. Python already renames this type of file simply "reset_beacons".
        if (boost::filesystem::is_regular_file(dirIter->path())) {
            resetBeacons = true;

            boost::filesystem::remove(dirIter->path());
        }
    }
    if (resetBeacons)
    {
        events.push_back(boost::shared_ptr<Event>(new ResetBeaconsEvent()));
    }

    return events;
}

int main(int argc, char *argv[])
{
    srand(time(0));

    boost::asio::io_service io_service;

    Listener listener(io_service);
    listener.startAccept();

    // server will scan this directory for pending deposits (supplied by py/balance_tracker.py)
    boost::filesystem::path depositsDirPath("./events_in/deposits/");
    boost::filesystem::directory_iterator directoryEndIter; // default constructor makes it an end_iter

    chrono::time_point<chrono::system_clock, chrono::duration<double>> nextFrameStart(chrono::system_clock::now());

    vector<WithdrawEvent> pendingWithdrawEvents;

    while (true)
    {
        // poll io_service, which will populate pendingCmds with anything the ClientChannels have received
        io_service.poll();

        // rate limit iteration to a maximum of SEC_PER_FRAME
        chrono::time_point<chrono::system_clock, chrono::duration<double>> now(chrono::system_clock::now());
        if (now < nextFrameStart)
            continue;
        nextFrameStart += ONE_FRAME;

        // let's count up events
        vector<boost::shared_ptr<Event>> pendingEvents;

        // did we see any withdrawals last loop?
        // If so, actuate and queue for in-game processing
        for (unsigned int i=0; i<pendingWithdrawEvents.size(); i++)
        {
            if (auto playerId = game.playerAddressToMaybeId(pendingWithdrawEvents[i].userAddress))
            {
                // just make sure again the math works out
                if (pendingWithdrawEvents[i].amountInCoins > game.players[*playerId].credit.getInt())
                {
                    cout << "Somehow an invalid withdrawal event was about to get processed..." << endl;
                }
                else
                {
                    actuateWithdrawal(pendingWithdrawEvents[i].userAddress, pendingWithdrawEvents[i].amountInCoins);
                    pendingEvents.push_back(pendingWithdrawEvents[i].toEventSharedPtr());
                }
            }
            else
            {
                cout << "Somehow trying to process an event for which there is no player..." << endl;
            }
        }
        pendingWithdrawEvents.clear();

        // scan for any pending deposits or honeypotAdd events
        vector<boost::shared_ptr<Event>> depositAndHoneypotEvents = pollPendingEvents();
        pendingEvents.insert(pendingEvents.end(), depositAndHoneypotEvents.begin(), depositAndHoneypotEvents.end());

        // build FrameEventsPacket for this frame
        // includes all cmds we've received from clients since last time and all new events
        FrameEventsPacket fep(game.frame, pendingCmds, pendingEvents);

        // send the packet out to all clients
        for (unsigned int i = 0; i < clientChannels.size(); i++)
        {
            if (clientChannels[i]->state == ClientChannel::DoingHandshake)
            {
                continue;
            }

            if (clientChannels[i]->state == ClientChannel::Closed)
            {
                clientChannels.erase(clientChannels.begin()+i);
                i--;
                continue;
            }

            bool sendResync = SEND_REGULAR_RESYNC_CHECKS ? (game.frame % 100 == 0) : false;
            if (clientChannels[i]->state == ClientChannel::ReadyForFirstSync || sendResync)
            {
                clientChannels[i]->sendResyncPacket();
                clientChannels[i]->state = ClientChannel::UpToDate;
            }

            if (clientChannels[i]->state == ClientChannel::UpToDate)
            {
                clientChannels[i]->sendFrameCmdsPacket(fep);
            }
            else
            {
                cout << "Unexpected behavior in server regarding ClientChannel state. Unexpected state: ";
                cout << (uint8_t)(clientChannels[i]->state) << endl;
            }
        }

        // execute all events
        for (unsigned int i = 0; i < pendingEvents.size(); i++)
        {
            pendingEvents[i]->execute(&game);
        }
        pendingEvents.clear();

        // execute all cmds on server-side game
        for (unsigned int i = 0; i < pendingCmds.size(); i++)
        {
            auto cmd = pendingCmds[i]->cmd;
            if (auto unitCmd = boost::dynamic_pointer_cast<UnitCmd, Cmd>(cmd))
            {
                unitCmd->executeAsPlayer(&game, pendingCmds[i]->playerAddress);
            }
            else if (auto spawnBeaconCmd = boost::dynamic_pointer_cast<SpawnBeaconCmd, Cmd>(cmd))
            {
                spawnBeaconCmd->executeAsPlayer(&game, pendingCmds[i]->playerAddress);
            }
            else if (auto withdrawCmd = boost::dynamic_pointer_cast<WithdrawCmd, Cmd>(pendingCmds[i]->cmd))
            {
                optional<uint8_t> maybePlayerId = game.playerAddressToMaybeId(pendingCmds[i]->playerAddress);
                if (!maybePlayerId)
                {
                    cout << "Woah, getting a negative playerId when processing a withdraw cmd..." << endl;
                    continue;
                }
                uint8_t playerId = *maybePlayerId;
                // if 0, interpret this as "all"
                coinsInt withdrawSpecified = withdrawCmd->amount > 0 ? withdrawCmd->amount : game.players[playerId].credit.getInt();
                coinsInt amountToWithdraw = min(withdrawSpecified, game.players[playerId].credit.getInt());

                pendingWithdrawEvents.push_back(WithdrawEvent(pendingCmds[i]->playerAddress, amountToWithdraw));
            }
            else
            {
                cout << "Woah, I don't know how to handle that cmd as a server!" << endl;
            }
        }
        pendingCmds.clear();

        game.iterate();
    }

    cout << "oohhhhh Logan you done did it this time" << endl;

    return 0;
}

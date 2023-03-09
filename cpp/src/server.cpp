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
#include "exec.h"

const bool SEND_REGULAR_RESYNC_CHECKS = true;

const unsigned int RECENT_BACKUP_INTERVAL_IN_FRAMES = 60*5; // every 5 seconds

const boost::filesystem::path COINFIGHT_DATA_DIR("/usr/share/coinfight_server/");
const boost::filesystem::path COINFIGHT_RUN_DIR("/var/run/coinfight/");
const boost::filesystem::path EVENTS_IN_DIR = COINFIGHT_RUN_DIR / "events_in";
const boost::filesystem::path EVENTS_OUT_DIR = COINFIGHT_RUN_DIR / "events_out";
const boost::filesystem::path SESSIONS_DATA_PATH = COINFIGHT_DATA_DIR / "sessions";

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
    boost::asio::streambuf receivedCodeHash;

    string sentChallenge;

    string genRandomString(int len)
    {
        // hacky, untested, probably insecure!!
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
        // Client initiates handshake by sending a code hash, to check version match
        receiveAndCheckCodeHashAsync();
    }

    void generateAndSendSigChallenge()
    {
        string challenge = genRandomString(50);

        boost::asio::write(*socket, boost::asio::buffer(challenge));

        cout << "sent challenge" << endl;

        sentChallenge = challenge;
    }

    void receiveAndCheckCodeHashAsync()
    {
        boost::asio::async_read_until(
            *socket,
            receivedCodeHash,
            '\n',
            boost::bind(
                &ClientChannel::codeHashReceived,
                this,
                boost::asio::placeholders::error,
                boost::asio::placeholders::bytes_transferred
            )
        );
    }

    void codeHashReceived(const boost::system::error_code &error, size_t transferred)
    {
        cout << "version received" << endl;
        if (error)
        {
            cout << "Error receiving codehash from " << connectionAuthdUserAddress.getString() << ". Kicking." << endl;
            state = Closed;
        }
        else
        {
            // turn the received data into a string, leaving out trailing \n.
            string codeHash(boost::asio::buffer_cast<const char*>(receivedCodeHash.data()), receivedCodeHash.size() - 1);

            // If there was a version mismatch, send a fail flag
            if (codeHash != GIT_COMMIT_HASH)
            {
                cout << "Received code hash doesn't match! Sending denial and closing connection." << endl;
                boost::asio::write(*socket, boost::asio::buffer(string("0")));
                state = Closed;
                return;
            }

            // Otherwise, send a success flag and a sig challenge
            cout << "Code hash matches. Sending success flag and sig challenge, and waiting for sig." << endl;
            boost::asio::write(*socket, boost::asio::buffer(string("1")));
            generateAndSendSigChallenge();

            // Wait for client's signature
            receiveSigAsync();
        }
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

            // now have sig and sentChallenge as strings.
            string error;
            if (auto maybeRecoveredAddress = signedMsgToAddressString(sentChallenge, sig, &error))
            {
                connectionAuthdUserAddress = *maybeRecoveredAddress;
                boost::asio::write(*socket, boost::asio::buffer(string("1")));
            }
            else
            {
                // send a 0 to indicate failure to client
                boost::asio::write(*socket, boost::asio::buffer(string("0")));
                cout << "Error recovering address from connection. Kicking." << endl << "Here's the Python error message:" << endl;
                cout << error << endl;
                state = Closed;
                return;
            }
            

            cout << "Player authenticated and connected." << endl;

            string response = connectionAuthdUserAddress.getString(); // prepend with 1 to indicate success
            boost::asio::write(*socket, boost::asio::buffer(response));

            state = ReadyForFirstSync;
            startReceivingLoop();
        }
    }

    void sendResyncPacket(Game* game)
    {
        packetsToSend.push_back(new vch);

        clearVchAndBuildResyncPacket(packetsToSend.back(), game);

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
            return;
        }

        Netpack::Consumer source(receivedBytes.begin());

        optional<boost::shared_ptr<Cmd>> maybeCmd = consumeCmd(&source);
        if (!maybeCmd)
        {
            cout << "Unrecognized command from user " << connectionAuthdUserAddress.getString() << ". Kicking." << endl;
            state = Closed;
            return;
        }
    
        boost::shared_ptr<AuthdCmd> authdCmd = boost::shared_ptr<AuthdCmd>(new AuthdCmd(*maybeCmd, this->connectionAuthdUserAddress));

        pendingCmds.push_back(authdCmd);

        clearVchAndReceiveNextCmd();
    }
};

void Listener::handleAccept(boost::shared_ptr<tcp::socket> socket, const boost::system::error_code &error)
{
    if (error)
    {
        cout << "Listener error accepting:" << error.value() << endl;
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

void actuateWithdrawal(Address userAddress, coinsInt amount)
{
    string weiString = coinsIntToWeiDepositString(amount);
    string writeData = userAddress.getString() + " " + weiString;

    string filename = to_string(time(0)) + "-" + to_string(clock());
    ofstream withdrawDescriptorFile("/tmp/coinfight/" + filename);
    withdrawDescriptorFile << writeData;
    withdrawDescriptorFile.close();

    boost::filesystem::copy("/tmp/coinfight/" + filename, EVENTS_OUT_DIR / "withdrawals" / filename);
    boost::filesystem::remove("/tmp/coinfight/" + filename);
}

tuple<bool, vector<boost::shared_ptr<Event>>> pollPendingEvents()
{
    vector<boost::shared_ptr<Event>> events;

    boost::filesystem::path depositsDirPath = EVENTS_IN_DIR / "deposits";
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
                        events.push_back(boost::shared_ptr<Event>(new DepositEvent(Address(userAddressOrHoneypotString), depositInCoins)));
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

    
    directoryEndIter = boost::filesystem::directory_iterator(); // default constructor makes it an end_iter

    bool resetBeacons = false;
    bool quitNow = false;
    for (boost::filesystem::directory_iterator dirIter(EVENTS_IN_DIR); dirIter != directoryEndIter; dirIter++)
    {
        if (boost::filesystem::is_regular_file(dirIter->path()))
        {
            bool consumeFile = true;

            string fileName = dirIter->path().filename().string();
            if (fileName == "reset_beacons")
            {
                resetBeacons = true;
                break;
            }
            else if (fileName == "halt")
            {
                quitNow = true;
            }
            else if (fileName == "end_game")
            {
                quitNow = true;
                cout << "WARNING: still need to code in a graceful exit." << endl;
            }
            else
            {
                cout << "unrecognized file: " << fileName << endl;
                consumeFile = false;
            }
            

            if (consumeFile)
            {
                boost::filesystem::remove(dirIter->path());
            }
        }
    }

    if (resetBeacons)
    {
        events.push_back(boost::shared_ptr<Event>(new ResetBeaconsEvent()));
    }

    return {quitNow, events};
}

class SessionSaver
{
    boost::filesystem::path rootDir, stateDir, fepsDir, stateBackupsDir, recentStateFile;
    void setupSessionDirectory()
    {
        string codeVersionTag = GIT_COMMIT_HASH;
        int sessionNumForVersion = 0;
        boost::filesystem::path attemptedSessionDirPath;
        do
        {
            stringstream sessionNameSS;
            sessionNameSS << codeVersionTag << "_" << sessionNumForVersion;

            attemptedSessionDirPath = SESSIONS_DATA_PATH / sessionNameSS.str();

            sessionNumForVersion ++;
        }
        while (boost::filesystem::exists(attemptedSessionDirPath));

        rootDir = attemptedSessionDirPath;
        stateDir = rootDir / "state";
        fepsDir = rootDir / "feps";
        recentStateFile = stateDir / "recent.cfs";
        stateBackupsDir = stateDir / "backups";

        boost::filesystem::create_directory(rootDir);
        boost::filesystem::create_directory(stateDir);
        boost::filesystem::create_directory(stateBackupsDir);
        boost::filesystem::create_directory(fepsDir);
    }

    Netpack::vch dataBuffer;

    void writeBufferToFile(boost::filesystem::path path)
    {
        boost::filesystem::ofstream fstream(path, ios::out | ios::binary);
        fstream.write((char*)(&dataBuffer[0]), dataBuffer.size());
        fstream.close();
    }
public:
    SessionSaver()
    {
        setupSessionDirectory();
    }
    void saveState(boost::filesystem::path path, Game* game)
    {
        dataBuffer.clear();
        Netpack::Builder b(&dataBuffer);
        game->pack(&b);

        writeBufferToFile(path);
    }
    void saveStateAsRecent(Game* game)
    {
        saveState(recentStateFile, game);
    }
    void saveFep(FrameEventsPacket* fep)
    {
        stringstream ss;
        ss << fep->frame << ".fep";
        boost::filesystem::path path = fepsDir / ss.str();

        dataBuffer.clear();
        Netpack::Builder b(&dataBuffer);
        fep->pack(&b);

        writeBufferToFile(path);
    }
};

bool loadState(boost::filesystem::path path, Game* game)
{
    if (! boost::filesystem::exists(path))
    {
        return false;
    }
    boost::filesystem::ifstream ifstream(path, ios::in | ios::binary);
    Netpack::vch dataBuffer = Netpack::vch(istreambuf_iterator<char>(ifstream), istreambuf_iterator<char>());
    ifstream.close();
    
    Netpack::vchIter iter = dataBuffer.begin();
    Netpack::Consumer c(iter);
    *game = Game(&c);
    game->reassignEntityGamePointers();

    return true;
}

boost::filesystem::path restoreLabelToStateFile(string restoreLabel)
{
    return SESSIONS_DATA_PATH / restoreLabel / "state" / "recent.cfs";
}

int main(int argc, char *argv[])
{
    boost::filesystem::create_directory("/tmp/coinfight/");

    optional<string> restoreLabel;
    optional<time_t> gameStartTime;

    int c;
    while ((c = getopt(argc, argv, "r:t:")) != -1)
    {
        switch (c)
        {
            case 'r':
            {
                restoreLabel = string(optarg);
                boost::trim(*restoreLabel);
                break;
            }
            case 't':
            {
                string timeStr(optarg);
                if (timeStr == "now")
                {
                    gameStartTime = time(NULL);
                }
                else
                {
                    gameStartTime = {stoi(timeStr)};
                }
                break;
            }
        }
    }

    if (!gameStartTime)
    {
        cout << "WARNING: no match start time set! Defaulting to now." << endl;
        gameStartTime = {time(NULL)};
    }

    int randSeed = time(NULL);
    Game game(randSeed, *gameStartTime);
    if (restoreLabel)
    {
        boost::filesystem::path statePath = restoreLabelToStateFile(*restoreLabel);
        cout << "restoring game state from " << statePath << endl;
        if (! loadState(statePath, &game))
        {
            throw runtime_error("Game state file not found");
        }
    }
    
    srand(time(0));

    SessionSaver sessionSaver;

    boost::asio::io_service io_service;

    Listener listener(io_service);
    listener.startAccept();

    // server will scan this directory for pending deposits (supplied by py/balance_tracker.py)
    boost::filesystem::path depositsDirPath = EVENTS_IN_DIR / "deposits";
    boost::filesystem::directory_iterator directoryEndIter; // default constructor makes it an end_iter

    chrono::time_point<chrono::system_clock, chrono::duration<double>> nextFrameStart(chrono::system_clock::now());

    vector<boost::shared_ptr<WithdrawEvent>> pendingWithdrawEvents;

    while (true)
    {
        if (game.frame != 0 && game.frame % RECENT_BACKUP_INTERVAL_IN_FRAMES == 0)
        {
            sessionSaver.saveStateAsRecent(&game);
        }
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
            if (auto playerId = game.playerAddressToMaybeId(pendingWithdrawEvents[i]->userAddress))
            {
                // just make sure again the math works out
                if (pendingWithdrawEvents[i]->amount > game.players[*playerId].credit.getInt())
                {
                    cout << "Somehow an invalid withdrawal event was about to get processed..." << endl;
                }
                else
                {
                    actuateWithdrawal(pendingWithdrawEvents[i]->userAddress, pendingWithdrawEvents[i]->amount);
                    pendingEvents.push_back(pendingWithdrawEvents[i]);
                }
            }
            else
            {
                cout << "Somehow trying to process an event for which there is no player..." << endl;
            }
        }
        pendingWithdrawEvents.clear();

        // scan for any pending deposits or honeypotAdd events
        auto quitNowAndEvents = pollPendingEvents();
        bool quitNow = get<0>(quitNowAndEvents);
        vector<boost::shared_ptr<Event>> depositAndHoneypotEvents = get<1>(quitNowAndEvents);

        if (quitNow)
        {
            sessionSaver.saveStateAsRecent(&game);
            return 0;
        }

        pendingEvents.insert(pendingEvents.end(), depositAndHoneypotEvents.begin(), depositAndHoneypotEvents.end());

        // build FrameEventsPacket for this frame
        // includes all cmds we've received from clients since last time and all new events
        FrameEventsPacket fep(game.frame, pendingCmds, pendingEvents);

        // If non-empty, save fep locally
        if (fep.events.size() > 0 || fep.authdCmds.size() > 0)
        {
            sessionSaver.saveFep(&fep);
        }

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
                clientChannels[i]->sendResyncPacket(&game);
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

                pendingWithdrawEvents.push_back(boost::shared_ptr<WithdrawEvent>(new WithdrawEvent(pendingCmds[i]->playerAddress, amountToWithdraw)));
            }
            else
            {
                cout << "Woah, I don't know how to handle that cmd as a server!" << endl;
            }
        }
        pendingCmds.clear();

        game.iterate();
    }

    return 0;
}
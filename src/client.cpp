#include <SFML/Graphics.hpp>
#include <GL/gl.h>
#include <GL/glu.h>
#include <iostream>
#include <boost/shared_ptr.hpp>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <vector>
#include <string>
#include "config.h"
#include "cmds.h"
#include "engine.h"
#include "graphics.h"
#include "common.h"

using namespace std;
using namespace boost::asio::ip;

void glEnable2D()
{
    GLint iViewport[4];

    //get a copy of the viewport
    glGetIntegerv(GL_VIEWPORT, iViewport);

    //save a copy of the projection matrix so we can restore it
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();

    //load identity projection matrix
    glLoadIdentity();

    //set up orthographic projection
    glOrtho(iViewport[0], iViewport[0] + iViewport[2], iViewport[1] + iViewport[3], iViewport[1], -1, 1);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    //ensure lighting and depth testing are disabled
    glPushAttrib(GL_DEPTH_BUFFER_BIT | GL_LIGHTING_BIT);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_LIGHTING);
}

void glDisable2D()
{
    glPopAttrib();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
}

Game game;

vector<FrameCmdsPacket> receivedFrameCmdsPackets;
vector<Game> receivedResyncs;

void clearVchAndBuildCmdPacket(vch *dest, boost::shared_ptr<Cmd> cmd)
{
    dest->clear();

    packTypechar(dest, cmd->getTypechar());
    cmd->pack(dest);

    vch prepended;
    packToVch(&prepended, "H", (uint16_t)(dest->size()));

    dest->insert(dest->begin(), prepended.begin(), prepended.end());
}

class ConnectionHandler
{
    vch receivedBytes;
    boost::asio::io_service &ioService;
    tcp::socket &socket;

    vector<vch *> packetsToSend;
    bool sending;

public:
    ConnectionHandler(boost::asio::io_service &ioService, tcp::socket &socket)
        : ioService(ioService), socket(socket)
    {
        sending = false;
    }
    void startReceiving()
    {
        clearVchAndReceiveNextPacket();
    }

    void clearVchAndReceiveNextPacket()
    {
        receivedBytes = vch(9);

        async_read(socket,
                   boost::asio::buffer(receivedBytes),
                   boost::bind(&ConnectionHandler::typecharAndSizeReceived,
                               this,
                               boost::asio::placeholders::error,
                               boost::asio::placeholders::bytes_transferred));
    }
    void typecharAndSizeReceived(const boost::system::error_code &error, size_t received)
    {
        if (!error)
        {
            vchIter place = receivedBytes.begin();

            unsigned char packetTypechar;
            uint64_t size;
            unpackFromIter(place, "CQ", &packetTypechar, &size);

            switch (packetTypechar)
            {
            case RESYNC_PACKET_CHAR:
                clearVchAndReceiveResyncPacket(size);
                break;

            case FRAMECMDS_PACKET_CHAR:
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
    void clearVchAndReceiveResyncPacket(uint64_t size)
    {
        receivedBytes = vch(size);

        async_read(socket,
                   boost::asio::buffer(receivedBytes),
                   boost::bind(&ConnectionHandler::resyncPacketReceived,
                               this,
                               boost::asio::placeholders::error,
                               boost::asio::placeholders::bytes_transferred));
    }
    void clearVchAndReceiveFrameCmdsPacket(uint64_t size)
    {
        receivedBytes = vch(size);

        async_read(socket,
                   boost::asio::buffer(receivedBytes),
                   boost::bind(&ConnectionHandler::frameCmdsPacketReceived,
                               this,
                               boost::asio::placeholders::error,
                               boost::asio::placeholders::bytes_transferred));
    }
    void resyncPacketReceived(const boost::system::error_code &error, size_t received)
    {
        if (!error)
        {
            vchIter place = receivedBytes.begin();

            cout << "BYTES:" << endl;
            debugOutputVch(receivedBytes);
            cout << endl
                 << ":FIN" << endl;

            receivedResyncs.push_back(Game(&place));

            clearVchAndReceiveNextPacket();
        }
        else
        {
            throw runtime_error("Error when receiving resync packet");
        }
    }
    void frameCmdsPacketReceived(const boost::system::error_code &error, size_t received)
    {
        if (error)
        {
            throw runtime_error("Error receiving frameCmds packet:" + error.value());
        }
        else
        {
            vchIter place = receivedBytes.begin();

            receivedFrameCmdsPackets.push_back(FrameCmdsPacket(&place));

            clearVchAndReceiveNextPacket();
        }
    }

    void sendCmd(boost::shared_ptr<Cmd> cmd)
    {
        packetsToSend.push_back(new vch);

        clearVchAndBuildCmdPacket(packetsToSend.back(), cmd);

        sendNextPacketIfNotBusy();
    }
    void sendNextPacketIfNotBusy()
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
    void wrapUpSendingPacket(vch *sourceToDispose, const boost::system::error_code &error, size_t bytes_transferred)
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
};

boost::shared_ptr<Entity> getEntityAtScreenPos(vector2f screenPos)
{
    vector2f gamePos = screenPos; // this will have to be changed when the screen can move

    boost::shared_ptr<Entity> closestValidEntity;
    float closestValidEntityDistance;
    for (unsigned int i = 0; i < game.entities.size(); i++)
    {
        boost::shared_ptr<Entity> e = game.entities[i];
        if (e)
        {
            if (e->collidesWithPoint(gamePos))
            {
                float distance = (gamePos - e->pos).getMagnitude();
                if (!closestValidEntity || distance < closestValidEntityDistance)
                {
                    closestValidEntity = e;
                    closestValidEntityDistance = distance;
                }
            }
        }
    }
    return closestValidEntity;
}
vector2f mouseButtonToVec(sf::Event::MouseButtonEvent mEvent)
{
    return vector2f(mEvent.x, mEvent.y);
}

void display(sf::RenderWindow &window)
{
    window.clear(); // Clear the buffer

    // glClearColor(0, 0, 0, 0);
    // glClear(GL_COLOR_BUFFER_BIT);
    // glColor3f(0, 1, 0);

    // glEnable2D();
    // glPushMatrix();

    for (unsigned int i = 0; i < game.entities.size(); i++)
    {
        if (game.entities[i])
            drawEntity(window, game.entities[i]);
    }

    // glTranslatef(50, 50, 0);

    // glBegin(GL_TRIANGLES);
    // glVertex2f(0, 0);
    // glVertex2f(10, 0);
    // glVertex2f(0, 10);
    // glEnd();

    // glPopMatrix();
    // glDisable2D();

    window.display(); // Update window contents
}

struct CommandUI
{
    vector<boost::shared_ptr<Entity>> selectedEntities;
} commandUI;

boost::shared_ptr<Cmd> makeAutoRightclickCmd(vector<boost::shared_ptr<Entity>> selectedEntities, boost::shared_ptr<Entity> targetedEntity)
{
    // Get typechar of units if they are all of same type
    unsigned char unitTypechar = getMaybeNullEntityTypechar(selectedEntities[0]);
    bool allSameType = true;
    for (uint i = 0; i < selectedEntities.size(); i++)
    {
        if (selectedEntities[i]->typechar() != unitTypechar)
        {
            allSameType = false;
            break;
        }
    }

    if (allSameType)
    {
        if (unitTypechar == PRIME_TYPECHAR)
        {
            if (targetedEntity->typechar() == GOLDPILE_TYPECHAR)
        {
            return boost::shared_ptr<Cmd>(new PickupCmd(entityPointersToRefs(selectedEntities), targetedEntity->ref));
            }
            else if (targetedEntity->typechar() == GATEWAY_TYPECHAR)
            {
                return boost::shared_ptr<Cmd>(new SendGoldThroughGatewayCmd(entityPointersToRefs(selectedEntities), targetedEntity->ref));
            }
        }
    }

    return boost::shared_ptr<Cmd>(); // return null cmd
}

int main()
{
    boost::asio::io_service io_service;
    tcp::socket socket(io_service);

    cout << "Connecting..." << endl;
    socket.connect(tcp::endpoint(tcp::v4(), 8473));

    cout << "Creating ConnectionHandler..." << endl;

    ConnectionHandler connectionHandler(io_service, socket);

    connectionHandler.startReceiving();

    // Get the first resync packet
    while (true)
    {
        io_service.poll();

        if (receivedResyncs.size() > 0)
        {
            game = receivedResyncs[0];
            game.reassignEntityGamePointers();

            receivedResyncs.erase(receivedResyncs.begin());
            break;
        }
    }

    sf::RenderWindow window(sf::VideoMode(640, 480), "OpenGL Test", sf::Style::Close | sf::Style::Titlebar);
    sf::Event event;

    clock_t nextFrameStart = clock() + (CLOCKS_PER_SEC * SEC_PER_FRAME);

    while (window.isOpen())
    {
        io_service.poll();

        if (clock() < nextFrameStart || receivedFrameCmdsPackets.size() == 0)
        {
            continue;
        }

        nextFrameStart += (CLOCKS_PER_SEC * SEC_PER_FRAME);

        boost::shared_ptr<Cmd> cmdToSend;

        while (window.pollEvent(event))
        {
            switch (event.type)
            {
            case sf::Event::Closed:
                window.close();
                break;
            case sf::Event::MouseButtonPressed:
                if (event.mouseButton.button == sf::Mouse::Left)
                {

                    if (boost::shared_ptr<Entity> clickedEntity = getEntityAtScreenPos(mouseButtonToVec(event.mouseButton)))
                    {
                        commandUI.selectedEntities.clear();
                        commandUI.selectedEntities.push_back(clickedEntity);
                    }
                }
                else if (event.mouseButton.button == sf::Mouse::Right && commandUI.selectedEntities.size() > 0)
                {
                    if (boost::shared_ptr<Entity> clickedEntity = getEntityAtScreenPos(mouseButtonToVec(event.mouseButton)))
                    {
                        cmdToSend = makeAutoRightclickCmd(commandUI.selectedEntities, clickedEntity);
                    }
                    else
                    {
                        if (boost::shared_ptr<Gateway> gateway = boost::dynamic_pointer_cast<Gateway, Entity>(commandUI.selectedEntities[0]))
                        {
                            // cmdToSend = new "start spawning" command...
                        }
                        else
                        {
                            cmdToSend = boost::shared_ptr<Cmd>(new MoveCmd(entityPointersToRefs(commandUI.selectedEntities), mouseButtonToVec(event.mouseButton)));
                        }
                    }
                }
                else if (event.mouseButton.button == sf::Mouse::Middle)
                {
                    cmdToSend = boost::shared_ptr<Cmd>(new PutdownCmd(entityPointersToRefs(commandUI.selectedEntities), Target(mouseButtonToVec(event.mouseButton))));
                }
                break;
            case sf::Event::KeyPressed:
                // cmdToSend = boost::shared_ptr<Cmd>(new PickupCmd(3, 2));
                break;
            default:
                break;
            }
        }

        if (cmdToSend)
        {
            connectionHandler.sendCmd(cmdToSend);
        }

        if (receivedResyncs.size() > 0 && receivedResyncs[0].frame == game.frame)
        {
            game = receivedResyncs[0];
            game.reassignEntityGamePointers();

            receivedResyncs.erase(receivedResyncs.begin());
        }

        FrameCmdsPacket fcp = receivedFrameCmdsPackets[0];
        receivedFrameCmdsPackets.erase(receivedFrameCmdsPackets.begin());

        assert(fcp.frame == game.frame);

        for (unsigned int i = 0; i < fcp.cmds.size(); i++)
        {
            fcp.cmds[i]->execute(&game);
        }

        game.iterate();

        display(window);

        if (game.frame % 200 == 0)
            cout << "num ncps " << receivedFrameCmdsPackets.size() << endl;
    }

    socket.close();

    return 0;
}
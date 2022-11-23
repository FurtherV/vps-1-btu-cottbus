#ifndef LIFECLIENT_H
#define LIFECLIENT_H

#include "board/LocalBoard.h"
#include "misc/Log.h"
#include "net/BarrierMessage.h"
#include "net/BoardGetMessage.h"
#include "net/BoardSetMessage.h"
#include "net/IPAddress.h"
#include "net/IPNetwork.h"
#include "net/LogonMessage.h"

/*
 * This class represents a LifeClient which connects to a boardserver, and then
 * calculates the status of determined Boardelements. For communication a network class is used.
 */
class LifeClient {
  public:
    /*
     * Constructor gets a pointer to a IPNetwork object for communication,
     * additionally the contact data for the server.
     *
     * @param net is the network object for communication
     * @param servername is the name of the server for example "ep", not an ip address in ascii-representation
     * @param port is the portnumber the boardserver listens
     */
    LifeClient(IPNetwork *net, const char *servername, short port) : net(net), server(IPAddress(servername, port)){};

    ~LifeClient(){};

    /**
     * This function contacts the server and gets the outlines of the boarpart which should be worked on.
     *
     * @return a -1 if logon failed, else 0
     */
    int start() {
        char buffer[100];
        LOG(INFO) << "Loggin into server...";
        LogonMessage *request = LogonMessage::createRequest(getNextSequenceNumber());
        ssize_t received_bytes = net->request(server, request, sizeof(LogonMessage), buffer, sizeof(buffer));
        delete request;
        LogonMessage *result = (LogonMessage *)buffer;
        client_id = result->client_id;
        timesteps = result->timesteps;
        x1 = result->start_x;
        y1 = result->start_y;
        x2 = result->end_x;
        y2 = result->end_y;
        LOG(INFO) << "[CLIENT-" << client_id << "] "
                  << "Login completed";
        return received_bytes > 0 ? 0 : -1;
    };

    /**
     * Loops until server signals end of the simulation, and calls makeStep() for the next simulation cycle
     */
    void loop() {
        while (timestep < timesteps) {
            LOG(INFO) << "[CLIENT-" << client_id << "] "
                      << "Simulating cycle " << timestep;
            makeStep();
            LOG(INFO) << "[CLIENT-" << client_id << "] "
                      << "Signaling doneness to server";
            char buffer[100];
            BarrierMessage *request = BarrierMessage::createRequest(getNextSequenceNumber(), client_id, timestep);
            net->request(server, request, sizeof(BarrierMessage), buffer, sizeof(buffer));
            timestep++;
        }
    };

  private:
    unsigned int sequence_number;
    int client_id;
    int timestep = 0;
    int timesteps;
    int x1, x2, y1, y2; // outlines the part of the board which should be calculated by the client
    IPNetwork *net;
    IPAddress server;

    /**
     * Calculate status for the next timestep on the given boardpart
     */
    void makeStep() {
        int width = (x2 - x1) + 2;
        int height = (y2 - y1) + 2;
        LocalBoard *board = new LocalBoard(width, height);
        board->clear();
        // read remote board
        for (int x = 0; x < board->getWidth(); x++) {
            for (int y = 0; y < board->getHeight(); y++) {
                board->setPos(x, y, getRemotePos(x + x1 - 1, y + y1 - 1));
            }
        }
        // do calculation
        board->step();
        // write remote board
        for (int x = 1; x < board->getWidth() - 1; x++) {
            for (int y = 1; y < board->getHeight() - 1; y++) {
                setRemotePos(x + x1 - 1, y + y1 - 1, board->getPos(x, y));
            }
        }
        delete board;
    };

    life_status_t getRemotePos(int x, int y) {
        char buffer[100];
        BoardGetMessage *request = BoardGetMessage::createRequest(getNextSequenceNumber(), x, y);
        net->request(server, request, sizeof(BoardGetMessage), buffer, sizeof(buffer));
        BoardGetMessage *result = (BoardGetMessage *)buffer;
        delete request;
        return result->state;
    }

    bool setRemotePos(int x, int y, life_status_t status) {
        char buffer[100];
        BoardSetMessage *request = BoardSetMessage::createRequest(getNextSequenceNumber(), x, y, status);
        net->request(server, request, sizeof(BoardSetMessage), buffer, sizeof(buffer));
        BoardSetMessage *result = (BoardSetMessage *)buffer;
        delete request;
        return result->confirmed;
    }

    unsigned int getNextSequenceNumber() { return sequence_number++; }
};

#endif

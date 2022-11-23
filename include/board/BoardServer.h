#ifndef BOARDSERVER_H
#define BOARDSERVER_H

#include <vector>

#include "board/Board.h"
#include "misc/Log.h"
#include "net/BarrierMessage.h"
#include "net/BoardGetMessage.h"
#include "net/BoardSetMessage.h"
#include "net/IPAddress.h"
#include "net/IPNetwork.h"
#include "net/LogonMessage.h"
#include "net/Message.h"
#include <bits/stdc++.h>

struct ClientInfo {
    int client_id = -1;
    IPAddress *address = nullptr;
    unsigned int last_sequence_number = 0;
    int last_completed_timestep = -1;
};

class BoardServer {
  public:
    /**
     * Constructor gets an IPNetwork over which communication will take
     * place.
     *
     * @param net is a pointer to a network object for communication
     * @param clientcount is the number of clients which the server requires
     * @param board_read is just a board
     * @param board_write is just another board, but must be same size as the previous board
     * @param timesteps is the amount simulation cycles
     */
    BoardServer(IPNetwork *net, size_t client_count, Board *board_read, Board *board_write, int timesteps = 0)
        : net(net), client_count(client_count), board_read(board_read), board_write(board_write), timesteps(timesteps) {
        if (client_count > (size_t)board_read->getHeight()) {
            LOG(WARN) << "Too many clients specified, maximum amount for given board is " << board_read->getHeight();
            LOG(WARN) << "Reducing required clients to maximum amount";
            this->client_count = (size_t)board_read->getHeight();
        }

        if (client_count > (size_t)INT_MAX) {
            throw std::invalid_argument("'client_count' was too high, maxmimum is " + std::to_string(INT_MAX));
        }

        board_write->clear();
    };

    /**
     * Frees all memory, which was allocated with new inside this class.
     */
    ~BoardServer() {
        for (auto client : clients) {
            delete client->address;
            delete client;
        }
    };

    void start() {
        LOG(INFO) << "Server started, using exactly " << client_count << " client(s) to calculate " << timesteps
                  << " cycle(s)";

        IPAddress client_address = IPAddress();
        while (timestep < timesteps) {
            client_address.setAddr(0);
            client_address.setPort(0);

            // receive bytes from a client
            char buffer[100];
            bzero(buffer, sizeof(buffer));
            net->receive(client_address, buffer, sizeof(buffer));

            // convert bytes to a message
            Message *message = (Message *)buffer;
            unsigned int sequence_number = message->getSequenceNumber();

            // parse message
            message_type_t message_type = message->getType();

            switch (message_type) {
            case message_type_t::logon: {
                logon(new IPAddress(client_address), sequence_number);
                break;
            }
            case message_type_t::board_get: {
                BoardGetMessage *req = (BoardGetMessage *)buffer;
                BoardGetMessage *rep =
                    BoardGetMessage::createReply(sequence_number, req->pos_x, req->pos_y, board_read);
                net->reply(client_address, rep, sizeof(BoardGetMessage));
                delete rep;
                break;
            }
            case message_type_t::board_set: {
                BoardSetMessage *req = (BoardSetMessage *)buffer;
                board_write->setPos(req->pos_x, req->pos_y, req->state);
                BoardSetMessage *rep = BoardSetMessage::createReply(sequence_number);
                net->reply(client_address, rep, sizeof(BoardSetMessage));
                delete rep;
                break;
            }
            case message_type_t::barrier: {
                BarrierMessage *req = (BarrierMessage *)buffer;
                barrier(req->client_id, sequence_number, req->finished_timestep);
                break;
            }
            default: {
                LOG(DEBUG) << (ssize_t)message->getType() << " is not a handled message type";
                break;
            }
            }
        }
    };

  private:
    IPNetwork *net;                    // network object used for communication
    size_t client_count;               // amount of required clients
    Board *board_read;                 // read only for timestep n
    Board *board_write;                // write only for timestep n + 1
    int timestep = 0;                  // current timestep
    int timesteps;                     // how many timesteps we are going to simulate in total
    std::vector<ClientInfo *> clients; // list of clients

    /**
     * This function is called, when a client connects to the server for the first time.
     * It stores the address of the client and determines the part of the board the client
     * has to work on.
     */
    void logon(IPAddress *client_address, unsigned int login_sequence_number) {
        // register new client
        int client_id = (int)clients.size();
        ClientInfo *client_info = new ClientInfo();
        client_info->client_id = client_id;
        client_info->address = client_address;
        client_info->last_sequence_number = login_sequence_number;
        clients.push_back(client_info);

        // calculate managed area for client
        int start_x, start_y, end_x, end_y;
        size_t rows_per_client = (size_t)board_read->getHeight() / client_count;
        size_t remaining_rows = (size_t)board_read->getHeight() - (rows_per_client * client_count);
        bool is_last_client = (size_t)client_id >= client_count - 1;
        size_t rows_for_this_client = is_last_client ? rows_per_client + remaining_rows : rows_per_client;
        start_x = 0;
        start_y = rows_per_client * client_id;
        end_x = board_read->getWidth();
        end_y = start_y + rows_for_this_client;

        // send client confirmation
        LogonMessage *rep = LogonMessage::createReply(login_sequence_number, client_id, start_x, start_y, end_x, end_y,
                                                      this->timesteps);
        net->reply(*client_address, rep, sizeof(LogonMessage));
        delete rep;

        LOG(INFO) << "New client with id " << client_id << " and address " << client_address->getAddr() << ":"
                  << client_address->getPort() << " registered";
    };

    /**
     * This function is the global barrier for interstepsynchronization. All clients must call
     * this function, before any of them gets a reply from the server. The reply can tell a client to
     * calculate the next timestep or inform him of the end of simulation.
     */
    void barrier(int client_id, unsigned int barrier_sequence_number, int completed_timestep) {
        // validate client id
        if (client_id < 0 || (size_t)client_id >= clients.size()) {
            LOG(WARN) << "Received barrier message with invalid client id " << client_id;
            return;
        }

        clients[client_id]->last_sequence_number = barrier_sequence_number;
        clients[client_id]->last_completed_timestep = completed_timestep;

        LOG(INFO) << "Client with id " << client_id << " is done with step " << completed_timestep;

        // check if all clients are done
        if (clients.size() < client_count) {
            return;
        }

        for (auto client : clients) {
            if (client->last_completed_timestep < timestep) {
                return;
            }
        }

        // all clients are done, swap boards and signal clients to continue
        LOG(INFO) << "All clients have completed step " << timestep;
        timestep += 1;
        board_read->clear();
        for (int x = 0; x < board_write->getWidth(); x++) {
            for (int y = 0; y < board_write->getHeight(); y++) {
                board_read->setPos(x, y, board_write->getPos(x, y));
            }
        }
        board_write->clear();
        notifyAll();
    };

    /**
     * This function tells a client, that it can continue to work or that the end of the simulation is reached.
     */
    void notify(int client_id) {
        BarrierMessage *rep = BarrierMessage::createReply(clients[client_id]->last_sequence_number, client_id);
        net->reply(*clients[client_id]->address, rep, sizeof(BarrierMessage));
        delete rep;
    };

    /**
     * The more general notify, which sends the same notification to all clients.
     */
    void notifyAll() {
        for (ClientInfo *client : clients) {
            notify(client->client_id);
        }
    };
};

#endif

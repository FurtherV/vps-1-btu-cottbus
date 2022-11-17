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
        if (client_count == (size_t)-1) {
            throw std::invalid_argument("'client_count' was too high, maxmimum is " + std::to_string((size_t)-2));
        }
        board_write->clear();
    };

    /**
     * Frees all memory, which was allocated with new inside this class.
     */
    ~BoardServer(){};

    void start() {
        while (timestep < timesteps) {
            // receive bytes from a client
            IPAddress client_address;
            char buffer[100];
            bzero(buffer, sizeof(buffer));
            net->receive(client_address, buffer, sizeof(buffer));

            // convert bytes to a message
            Message *message = (Message *)buffer;
            unsigned int sequence_number = message->getSequenceNumber();

            // parse message
            message_type_t message_type = message->getType();
            // LOG(DEBUG) << "Message (" << (ssize_t)message->getType() << ") received from " <<
            // client_address.getAddr()
            //            << ":" << client_address.getPort() << " with sequence number " << sequence_number;

            switch (message_type) {
            case message_type_t::logon: {
                logon(new IPAddress(client_address), sequence_number);
                break;
            }
            case message_type_t::board_get: {
                BoardGetMessage *bg_message = (BoardGetMessage *)message;
                int x = bg_message->pos_x;
                int y = bg_message->pos_y;
                BoardGetMessage *reply = BoardGetMessage::createReply(sequence_number, x, y, board_read);
                net->reply(client_address, reply, sizeof(BoardGetMessage));
                delete reply;
                break;
            }
            case message_type_t::board_set: {
                BoardSetMessage *bs_message = (BoardSetMessage *)message;
                int x = bs_message->pos_x;
                int y = bs_message->pos_y;
                life_status_t state = bs_message->state;
                board_write->setPos(x, y, state);
                BoardSetMessage *reply = BoardSetMessage::createReply(sequence_number);
                net->reply(client_address, reply, sizeof(BoardSetMessage));
                delete reply;
                break;
            }
            case message_type_t::barrier: {
                BarrierMessage *b_message = (BarrierMessage *)message;
                int client_id = b_message->client_id;
                int finished_timestep = b_message->finished_timestep;
                client_barrier_sequence_number[client_id] = sequence_number;
                barrier(&client_address, finished_timestep);
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
    IPNetwork *net;                     // network object used for communication
    size_t client_count;                // amount of required clients
    Board *board_read;                  // read only for timestep n
    Board *board_write;                 // write only for timestep n + 1
    int timestep = 0;                   // current timestep
    int timesteps;                      // how many timesteps we are going to simulate in total
    std::vector<IPAddress *> addresses; // list of client addresses
    std::vector<int> client_timesteps;  // map of client id to client current timestep
    std::vector<unsigned int>
        client_barrier_sequence_number; // map of client id to sequence number used in barrier message

    /**
     * This function is called, when a client connects to the server for the first time.
     * It stores the address of the client and determines the part of the board the client
     * has to work on.
     */
    void logon(IPAddress *ip, int sequence_number) {
        // add client to lists, if joined late or rejoined they can still work with current timestep
        addresses.push_back(ip);
        client_timesteps.push_back(timestep);
        client_barrier_sequence_number.push_back(0);

        int client_id = (int)addresses.size() - 1;
        LOG(INFO) << "[LOGIN] Client:" << client_id << " from " << ip->getAddr() << ":" << ip->getPort();

        int start_x, start_y, end_x, end_y;
        if (client_count == 1) {
            start_x = 0;
            start_y = 0;
            end_x = board_read->getWidth();
            end_y = board_read->getHeight();
        } else {
            int rows_per_client = board_read->getHeight() / client_count;
            int rows_for_last_client = rows_per_client + (board_read->getHeight() % client_count);
            bool is_this_last_client = ((size_t)client_id == (client_count - 1));
            int rows_for_this_client = !is_this_last_client ? rows_per_client : rows_for_last_client;
            start_x = 0;
            start_y = rows_per_client * client_id;
            end_x = board_read->getWidth();
            end_y = start_y + rows_for_this_client;
        }

        LOG(INFO) << "[LOGIN] Client:" << client_id << " has area (" << start_x << "," << start_y << ") to (" << end_x
                  << "," << end_y << ")";

        LogonMessage *message =
            LogonMessage::createReply(sequence_number, client_id, start_x, start_y, end_x, end_y, timesteps);
        net->reply(*ip, message, sizeof(LogonMessage));
        delete message;
    };

    /**
     * This function is the global barrier for interstepsynchronization. All clients must call
     * this function, before any of them gets a reply from the server. The reply can tell a client to
     * calculate the next timestep or inform him of the end of simulation.
     *
     * @param ip is the address of the calling client
     * @param timestep is the finished timestep which the client is signalling
     */
    void barrier(IPAddress *ip, int timestep) {
        size_t index = getClientIdFromIP(ip);

        if (index == (size_t)-1) {
            LOG(WARN) << "Unkown client '" << ip->getAddr() << ":" << ip->getPort()
                      << "' tried to use barrier function";
            return;
        }

        LOG(INFO) << "[SIMULATION] [CYCLE-" << timestep << "] Client:" << index << " Done";

        client_timesteps[index] = timestep + 1;
        bool all_clients_done = true;
        if (client_timesteps.size() < client_count) {
            all_clients_done = false;
        } else {
            for (auto client_timestep : client_timesteps) {
                if (client_timestep <= this->timestep) {
                    all_clients_done = false;
                    break;
                }
            }
        }
        if (all_clients_done) {
            LOG(INFO) << "[SIMULATION] [CYCLE-" << timestep << "] Completed";
            // copy everything from board_write to board_read and clear board_write
            board_read->clear();
            for (int x = 0; x < board_read->getWidth(); x++) {
                for (int y = 0; y < board_read->getHeight(); y++) {
                    board_read->setPos(x, y, board_write->getPos(x, y));
                }
            }
            board_write->clear();

            this->timestep += 1;
            // must be done to trigger possible UI refresh
            board_read->setCurrentStep((size_t)timestep);
            board_write->setCurrentStep((size_t)timestep);
            notifyAll();
        }
    };

    /**
     * @brief Returns a clients id from their ip
     * @param ip ip of the client
     * @return On success, id of the client is returned. On failure, -1 is returned
     */
    size_t getClientIdFromIP(IPAddress *ip) {
        size_t index = (size_t)-1;
        for (size_t i = 0; i < addresses.size(); i++) {
            auto address = addresses[i];
            if ((*address) == (*ip)) {
                index = i;
                break;
            }
        }
        return index;
    }

    /**
     * This function tells a client, that it can continue to work or that the end of the simulation is reached.
     *
     * @param ip is the address of the client which should be notified
     */
    void notify(IPAddress *ip) {
        size_t index = getClientIdFromIP(ip);
        if (index == (size_t)-1)
            return;
        BarrierMessage *msg = BarrierMessage::createReply(client_barrier_sequence_number[index], index);
        net->reply(*ip, msg, sizeof(BarrierMessage));
        delete msg;
    };

    /**
     * The more general notify, which sends the same notification to all clients.
     */
    void notifyAll() {
        for (auto client : addresses) {
            notify(client);
        }
    };
};

#endif

#ifndef BOARDSERVER_H
#define BOARDSERVER_H

#include "board/Board.h"
#include "misc/Log.h"
#include "misc/Stopwatch.h"
#include "net/BarrierMessage.h"
#include "net/BoardGetMessage.h"
#include "net/BoardSetMessage.h"
#include "net/IPAddress.h"
#include "net/IPNetwork.h"
#include "net/LogonMessage.h"
#include "net/Message.h"
#include <bits/stdc++.h>
#include <vector>

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
    BoardServer(IPNetwork *net, size_t client_count, Board *board_read, Board *board_write, int timesteps = 0);

    /**
     * Frees all memory, which was allocated with new inside this class.
     */
    virtual ~BoardServer();

    void start();

    Stopwatch get_profiler() { return stopwatch; }

  private:
    IPNetwork *net;                    // network object used for communication
    size_t client_count;               // amount of required clients
    Board *board_read;                 // read only for timestep n
    Board *board_write;                // write only for timestep n + 1
    int timestep = 0;                  // current timestep
    int timesteps;                     // how many timesteps we are going to simulate in total
    std::vector<ClientInfo *> clients; // list of clients
    Stopwatch stopwatch;

    /**
     * This function is called, when a client connects to the server for the first time.
     * It stores the address of the client and determines the part of the board the client
     * has to work on.
     */
    void logon(IPAddress *client_address, unsigned int login_sequence_number);

    /**
     * This function is the global barrier for interstepsynchronization. All clients must call
     * this function, before any of them gets a reply from the server. The reply can tell a client to
     * calculate the next timestep or inform him of the end of simulation.
     */
    void barrier(int client_id, unsigned int barrier_sequence_number, int completed_timestep);

    /**
     * This function tells a client, that it can continue to work or that the end of the simulation is reached.
     */
    void notify(int client_id);

    /**
     * The more general notify, which sends the same notification to all clients.
     */
    void notifyAll();
};

#endif

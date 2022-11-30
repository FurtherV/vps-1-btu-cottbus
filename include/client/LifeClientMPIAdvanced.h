#ifndef LIFECLIENTMPIADVANCED_H
#define LIFECLIENTMPIADVANCED_H

#include "board/LocalBoard.h"
#include <mpi.h>

class LifeClientMPIAdvanced {
  public:
    /**
     * @brief Creates a client which will help a server simulate the Game of Life by simulating portions of the overall
     * board.
     * @param root_rank rank / id of the server, usually 0
     */
    LifeClientMPIAdvanced(int root_rank);

    virtual ~LifeClientMPIAdvanced();

    /**
     * @brief Starts the client.
     */
    void start();

  private:
    void receive_timesteps();

    void send_area();

    void receive_area(bool first_pass);

    void barrier();

    int timesteps;
    int current_timestep = 0;
    int root_rank = 0;
    int start_x, start_y = -1;
    int end_x, end_y = -1;
    LocalBoard *board = nullptr;
};

#endif
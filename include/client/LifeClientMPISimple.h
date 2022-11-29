#ifndef LIFECLIENTMPISIMPLE_H
#define LIFECLIENTMPISIMPLE_H

#include "board/LocalBoard.h"
#include <mpi.h>

class LifeClientMPISimple {
  public:
    LifeClientMPISimple(int root_rank);

    virtual ~LifeClientMPISimple();

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
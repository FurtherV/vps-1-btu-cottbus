#ifndef BOARDSERVERMPI_H
#define BOARDSERVERMPI_H

#include "board/Board.h"
#include "misc/Stopwatch.h"
#include <mpi.h>

class BoardServerMPI {
  public:
    /**
     * @brief Creates a server using two boards, BOARD_READ and BOARD_WRITE, which must have equal dimensions. The
     * server will simulate TIMESTEPS steps of the Game of Life.
     * @param board_read Board mostly used for reading, but also for writing
     * @param board_write Board only used for writing
     * @param timesteps Number of steps to simulate
     */
    BoardServerMPI(Board *board_read, Board *board_write, int timesteps);

    virtual ~BoardServerMPI();

    /**
     * @brief Starts the server with an optional stopwatch for profiling.
     * @param stopwatch Stopwatch used for profiling.
     */
    void start(Stopwatch *stopwatch = nullptr);

  private:
    void swap_boards();

    void broadcast_timesteps();

    void receive_areas();

    void receive_area(int rank);

    void send_areas(bool first_pass);

    void send_area(int rank, bool first_pass);

    void barrier();

    void calculate_area(int rank, int &start_x, int &start_y, int &end_x, int &end_y);

    Board *board_read;
    Board *board_write;
    int timesteps;
    int current_timestep = 0;
};

#endif
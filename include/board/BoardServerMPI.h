#ifndef BOARDSERVERMPI_H
#define BOARDSERVERMPI_H

#include "misc/Stopwatch.h"

/**
 * @brief Interface for a board server that uses MPI.
 */
class BoardServerMPI {
  public:
    virtual ~BoardServerMPI(){};

    /**
     * @brief Starts the server with an optional stopwatch for profiling.
     * @param stopwatch Stopwatch used for profiling.
     */
    virtual void start(Stopwatch *stopwatch = nullptr) = 0;
};

#endif
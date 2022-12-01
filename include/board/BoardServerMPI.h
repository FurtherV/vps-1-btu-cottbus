#ifndef BOARDSERVERMPI_H
#define BOARDSERVERMPI_H

/**
 * @brief Interface for a board server that uses MPI.
 */
class BoardServerMPI {
  public:
    virtual ~BoardServerMPI(){};

    /**
     * @brief Starts the server.
     */
    virtual void start() = 0;
};

#endif
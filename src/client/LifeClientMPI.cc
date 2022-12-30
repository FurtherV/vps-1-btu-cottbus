#include "client/LifeClientMPI.h"
#include "misc/Log.h"

LifeClientMPI::LifeClientMPI(int root_rank) : root_rank(root_rank) {}

LifeClientMPI::~LifeClientMPI() {
    if (board != nullptr) {
        delete board;
    }
}

void LifeClientMPI::start() {
    receive_timesteps();
    receive_area(true);
    while (current_timestep < timesteps) {
        board->step();
        barrier();
        send_area();
        barrier();
        receive_area(false);
        current_timestep++;
        board->setCurrentStep(current_timestep);
    }
}

void LifeClientMPI::receive_timesteps() { MPI_Bcast(&timesteps, 1, MPI_INT, root_rank, MPI_COMM_WORLD); }

void LifeClientMPI::send_area() {
    int buffer_size = 0;
    MPI_Pack_size((board->getWidth() - 2) * (board->getHeight() - 2), MPI_CHAR, MPI_COMM_WORLD, &buffer_size);

    char *buffer = new char[buffer_size];
    bzero(buffer, buffer_size);
    int pack_counter = 0;

    for (int x = 1; x < board->getWidth() - 1; x++) {
        for (int y = 1; y < board->getHeight() - 1; y++) {
            char life_state_byte = (char)board->getPos(x, y);
            MPI_Pack(&life_state_byte, 1, MPI_CHAR, buffer, buffer_size, &pack_counter, MPI_COMM_WORLD);
        }
    }

    MPI_Send(buffer, pack_counter, MPI_PACKED, root_rank, 3, MPI_COMM_WORLD);
    delete[] buffer;
}

void LifeClientMPI::receive_area(bool first_pass) {
    if (first_pass) {
        // receive board area and surroundings

        int buffer_size = 0;
        MPI_Pack_size(4, MPI_INT, MPI_COMM_WORLD, &buffer_size);

        char *buffer = new char[buffer_size];
        int unpack_counter = 0;

        MPI_Recv(buffer, buffer_size, MPI_PACKED, root_rank, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        MPI_Unpack(buffer, buffer_size, &unpack_counter, &start_x, 1, MPI_INT, MPI_COMM_WORLD);
        MPI_Unpack(buffer, buffer_size, &unpack_counter, &start_y, 1, MPI_INT, MPI_COMM_WORLD);
        MPI_Unpack(buffer, buffer_size, &unpack_counter, &end_x, 1, MPI_INT, MPI_COMM_WORLD);
        MPI_Unpack(buffer, buffer_size, &unpack_counter, &end_y, 1, MPI_INT, MPI_COMM_WORLD);

        int width = (end_x - start_x) + 2;
        int height = (end_y - start_y) + 2;
        board = new LocalBoard(width, height);
        board->clear();

        delete[] buffer;

        buffer_size = 0;
        MPI_Pack_size(board->getWidth() * board->getHeight(), MPI_CHAR, MPI_COMM_WORLD, &buffer_size);
        buffer = new char[buffer_size];
        unpack_counter = 0;

        MPI_Recv(buffer, buffer_size, MPI_PACKED, root_rank, 2, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        for (int x = 0; x < board->getWidth(); x++) {
            for (int y = 0; y < board->getHeight(); y++) {
                char life_state_byte = 0;
                MPI_Unpack(buffer, buffer_size, &unpack_counter, &life_state_byte, 1, MPI_CHAR, MPI_COMM_WORLD);
                board->setPos(x, y, (life_status_t)life_state_byte);
            }
        }

        delete[] buffer;
    } else {
        // receive surroundings of board area
        int buffer_size = 0;
        MPI_Pack_size(board->getWidth() * 2 + board->getHeight() * 2, MPI_CHAR, MPI_COMM_WORLD, &buffer_size);

        char *buffer = new char[buffer_size];
        bzero(buffer, buffer_size);
        int unpack_counter = 0;

        MPI_Recv(buffer, buffer_size, MPI_PACKED, root_rank, 2, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        for (int x = 0; x < board->getWidth(); x++) {
            char life_state_byte = 0;
            MPI_Unpack(buffer, buffer_size, &unpack_counter, &life_state_byte, 1, MPI_CHAR, MPI_COMM_WORLD);
            board->setPos(x, 0, (life_status_t)life_state_byte);

            life_state_byte = 0;
            MPI_Unpack(buffer, buffer_size, &unpack_counter, &life_state_byte, 1, MPI_CHAR, MPI_COMM_WORLD);
            board->setPos(x, board->getHeight() - 1, (life_status_t)life_state_byte);
        }

        for (int y = 0; y < board->getHeight(); y++) {
            char life_state_byte = 0;
            MPI_Unpack(buffer, buffer_size, &unpack_counter, &life_state_byte, 1, MPI_CHAR, MPI_COMM_WORLD);
            board->setPos(0, y, (life_status_t)life_state_byte);

            life_state_byte = 0;
            MPI_Unpack(buffer, buffer_size, &unpack_counter, &life_state_byte, 1, MPI_CHAR, MPI_COMM_WORLD);
            board->setPos(board->getWidth() - 1, y, (life_status_t)life_state_byte);
        }

        delete[] buffer;
    }
}

void LifeClientMPI::barrier() { MPI_Barrier(MPI_COMM_WORLD); }
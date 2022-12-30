#include "board/BoardServerMPI.h"
#include "misc/Log.h"

BoardServerMPI::BoardServerMPI(Board *board_read, Board *board_write, int timesteps)
    : board_read(board_read), board_write(board_write), timesteps(timesteps) {}

BoardServerMPI::~BoardServerMPI() {}

void BoardServerMPI::start(Stopwatch *stopwatch) {
    if (stopwatch != nullptr) {
        stopwatch->start();
    }

    broadcast_timesteps();
    send_areas(true);

    while (current_timestep < timesteps) {
        barrier();
        receive_areas();
        swap_boards();
        barrier();
        send_areas(false);

        current_timestep++;

        board_read->setCurrentStep(current_timestep);
        board_write->setCurrentStep(current_timestep);
        if (stopwatch != nullptr) {
            stopwatch->stop();
        }
    }
}

void BoardServerMPI::swap_boards() {
    board_read->clear();
    for (int x = 0; x < board_read->getWidth(); x++) {
        for (int y = 0; y < board_read->getHeight(); y++) {
            board_read->setPos(x, y, board_write->getPos(x, y));
        }
    }
    board_write->clear();
}

void BoardServerMPI::broadcast_timesteps() {
    int root = 0;
    MPI_Comm_rank(MPI_COMM_WORLD, &root);
    MPI_Bcast(&timesteps, 1, MPI_INT, root, MPI_COMM_WORLD);
}

void BoardServerMPI::receive_areas() {
    int size = 0;
    int my_rank = 0;

    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);

    for (int i = 0; i < size; i++) {
        if (i == my_rank)
            continue;
        receive_area(i);
    }
}

void BoardServerMPI::receive_area(int rank) {
    int start_x, start_y, end_x, end_y;
    calculate_area(rank, start_x, start_y, end_x, end_y);

    int width = end_x - start_x;
    int height = end_y - start_y;

    int buffer_size = 0;
    MPI_Pack_size(width * height, MPI_CHAR, MPI_COMM_WORLD, &buffer_size);

    char *buffer = new char[buffer_size];
    bzero(buffer, buffer_size);
    MPI_Recv(buffer, buffer_size, MPI_PACKED, rank, 3, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

    int unpack_count = 0;
    for (int x = start_x; x < end_x; x++) {
        for (int y = start_y; y < end_y; y++) {
            char life_state_byte = 0;
            MPI_Unpack(buffer, buffer_size, &unpack_count, &life_state_byte, 1, MPI_CHAR, MPI_COMM_WORLD);
            board_write->setPos(x, y, (life_status_t)life_state_byte);
        }
    }

    delete[] buffer;
}

void BoardServerMPI::send_areas(bool first_pass) {
    int size = 0;
    int my_rank = 0;

    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);

    for (int i = 0; i < size; i++) {
        if (i == my_rank)
            continue;
        send_area(i, first_pass);
    }
}

void BoardServerMPI::send_area(int rank, bool first_pass) {
    int start_x, start_y, end_x, end_y;
    calculate_area(rank, start_x, start_y, end_x, end_y);

    int width = end_x - start_x;
    int height = end_y - start_y;

    if (first_pass) {
        // send board area and surroundings

        int buffer_size = 0;
        MPI_Pack_size(4, MPI_INT, MPI_COMM_WORLD, &buffer_size);

        char *buffer = new char[buffer_size];
        bzero(buffer, buffer_size);
        int pack_counter = 0;

        MPI_Pack(&start_x, 1, MPI_INT, buffer, buffer_size, &pack_counter, MPI_COMM_WORLD);
        MPI_Pack(&start_y, 1, MPI_INT, buffer, buffer_size, &pack_counter, MPI_COMM_WORLD);
        MPI_Pack(&end_x, 1, MPI_INT, buffer, buffer_size, &pack_counter, MPI_COMM_WORLD);
        MPI_Pack(&end_y, 1, MPI_INT, buffer, buffer_size, &pack_counter, MPI_COMM_WORLD);

        MPI_Send(buffer, pack_counter, MPI_PACKED, rank, 1, MPI_COMM_WORLD);
        delete[] buffer;

        buffer_size = 0;
        MPI_Pack_size((width + 2) * (height + 2), MPI_CHAR, MPI_COMM_WORLD, &buffer_size);

        buffer = new char[buffer_size];
        bzero(buffer, buffer_size);
        pack_counter = 0;
        for (int x = start_x - 1; x < end_x + 1; x++) {
            for (int y = start_y - 1; y < end_y + 1; y++) {
                char life_state_byte = (char)board_read->getPos(x, y);
                MPI_Pack(&life_state_byte, 1, MPI_CHAR, buffer, buffer_size, &pack_counter, MPI_COMM_WORLD);
            }
        }
        MPI_Send(buffer, pack_counter, MPI_PACKED, rank, 2, MPI_COMM_WORLD);
        delete[] buffer;
    } else {
        // send just the surroundings of the board area

        int buffer_size = 0;
        MPI_Pack_size((width + 2) * 2 + (height + 2) * 2, MPI_CHAR, MPI_COMM_WORLD, &buffer_size);

        char *buffer = new char[buffer_size];
        bzero(buffer, buffer_size);
        int pack_counter = 0;

        for (int x = start_x - 1; x < end_x + 1; x++) {
            char life_state_byte_up = (char)board_read->getPos(x, start_y - 1);
            char life_state_byte_down = (char)board_read->getPos(x, end_y);
            MPI_Pack(&life_state_byte_up, 1, MPI_CHAR, buffer, buffer_size, &pack_counter, MPI_COMM_WORLD);
            MPI_Pack(&life_state_byte_down, 1, MPI_CHAR, buffer, buffer_size, &pack_counter, MPI_COMM_WORLD);
        }

        for (int y = start_y - 1; y < end_y + 1; y++) {
            char life_state_byte_left = (char)board_read->getPos(start_x - 1, y);
            char life_state_byte_right = (char)board_read->getPos(end_x, y);
            MPI_Pack(&life_state_byte_left, 1, MPI_CHAR, buffer, buffer_size, &pack_counter, MPI_COMM_WORLD);
            MPI_Pack(&life_state_byte_right, 1, MPI_CHAR, buffer, buffer_size, &pack_counter, MPI_COMM_WORLD);
        }

        MPI_Send(buffer, pack_counter, MPI_PACKED, rank, 2, MPI_COMM_WORLD);
        delete[] buffer;
    }
}

void BoardServerMPI::barrier() { MPI_Barrier(MPI_COMM_WORLD); }

void BoardServerMPI::calculate_area(int rank, int &start_x, int &start_y, int &end_x, int &end_y) {
    // rows are evenly distributed among clients.
    // If it can not be evenly distributed, early clients get a row more than later clients.
    // Example: 100 rows, 7 clients, 0 = 15, 1 = 15, 2 to 6 = 14

    int comm_world_size = 0;
    MPI_Comm_size(MPI_COMM_WORLD, &comm_world_size);

    int client_id = rank - 1;
    int clients = comm_world_size - 1;
    int rows = board_read->getHeight();

    int rows_per_client = rows / clients;
    int rows_per_client_remainder = rows % clients;

    int already_used_rows = client_id * rows_per_client + min(client_id, rows_per_client_remainder);
    int rows_used_by_this_client = rows_per_client + (client_id < rows_per_client_remainder ? 1 : 0);

    start_x = 0;
    start_y = already_used_rows;
    end_x = board_read->getWidth();
    end_y = start_y + rows_used_by_this_client;
}
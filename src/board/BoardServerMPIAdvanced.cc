#include "board/BoardServerMPIAdvanced.h"
#include "misc/Log.h"
#include "misc/Stopwatch.h"

BoardServerMPIAdvanced::BoardServerMPIAdvanced(Board *board_read, Board *board_write, int timesteps)
    : board_read(board_read), board_write(board_write), timesteps(timesteps) {}

BoardServerMPIAdvanced::~BoardServerMPIAdvanced() {}

void BoardServerMPIAdvanced::start(Stopwatch *stopwatch) {
    if (stopwatch != nullptr) {
        stopwatch->start();
    }

    broadcast_timesteps();
    LOG(INFO) << "[SERVER] "
              << "Sending initial areas";
    send_areas(true);

    while (current_timestep < timesteps) {

        LOG(INFO) << "[SERVER] "
                  << "Waiting on clients...";
        barrier();

        LOG(INFO) << "[SERVER] "
                  << "Receiving areas";
        receive_areas();
        swap_boards();

        LOG(INFO) << "[SERVER] "
                  << "Waiting on clients...";
        barrier();

        LOG(INFO) << "[SERVER] "
                  << "Sending areas";
        send_areas(false);

        current_timestep++;

        board_read->setCurrentStep(current_timestep);
        board_write->setCurrentStep(current_timestep);
        if (stopwatch != nullptr) {
            stopwatch->stop();
        }
    }
    LOG(INFO) << "[SERVER] "
              << "Simulation done.";
}

void BoardServerMPIAdvanced::swap_boards() {
    board_read->clear();
    for (int x = 0; x < board_read->getWidth(); x++) {
        for (int y = 0; y < board_read->getHeight(); y++) {
            board_read->setPos(x, y, board_write->getPos(x, y));
        }
    }
    board_write->clear();
}

void BoardServerMPIAdvanced::broadcast_timesteps() {
    MPI::COMM_WORLD.Bcast(&timesteps, 1, MPI::INT, MPI::COMM_WORLD.Get_rank());
}

void BoardServerMPIAdvanced::receive_areas() {
    for (int i = 0; i < MPI::COMM_WORLD.Get_size(); i++) {
        if (i == MPI::COMM_WORLD.Get_rank())
            continue;
        receive_area(i);
    }
}

void BoardServerMPIAdvanced::receive_area(int rank) {
    int start_x, start_y, end_x, end_y;
    calculate_area(rank, start_x, start_y, end_x, end_y);

    int width = end_x - start_x;
    int height = end_y - start_y;

    int buffer_size = MPI::CHAR.Pack_size(width * height, MPI::COMM_WORLD);
    char *buffer = new char[buffer_size];
    bzero(buffer, buffer_size);
    MPI::COMM_WORLD.Recv(buffer, buffer_size, MPI::PACKED, rank, 3);

    int unpack_count = 0;
    for (int x = start_x; x < end_x; x++) {
        for (int y = start_y; y < end_y; y++) {
            char life_state_byte = 0;
            MPI::CHAR.Unpack(buffer, buffer_size, &life_state_byte, 1, unpack_count, MPI::COMM_WORLD);
            board_write->setPos(x, y, (life_status_t)life_state_byte);
        }
    }

    delete[] buffer;
}

void BoardServerMPIAdvanced::send_areas(bool first_pass) {
    for (int i = 0; i < MPI::COMM_WORLD.Get_size(); i++) {
        if (i == MPI::COMM_WORLD.Get_rank())
            continue;
        send_area(i, first_pass);
    }
}

void BoardServerMPIAdvanced::send_area(int rank, bool first_pass) {
    int start_x, start_y, end_x, end_y;
    calculate_area(rank, start_x, start_y, end_x, end_y);

    int width = end_x - start_x;
    int height = end_y - start_y;

    if (first_pass) {
        // send board area and surroundings

        int buffer_size = MPI::INT.Pack_size(4, MPI::COMM_WORLD);
        char *buffer = new char[buffer_size];
        bzero(buffer, buffer_size);
        int pack_counter = 0;
        MPI::INT.Pack(&start_x, 1, buffer, buffer_size, pack_counter, MPI::COMM_WORLD);
        MPI::INT.Pack(&start_y, 1, buffer, buffer_size, pack_counter, MPI::COMM_WORLD);
        MPI::INT.Pack(&end_x, 1, buffer, buffer_size, pack_counter, MPI::COMM_WORLD);
        MPI::INT.Pack(&end_y, 1, buffer, buffer_size, pack_counter, MPI::COMM_WORLD);
        MPI::COMM_WORLD.Send(buffer, pack_counter, MPI::PACKED, rank, 1);
        delete[] buffer;

        buffer_size = MPI::CHAR.Pack_size((width + 2) * (height + 2), MPI::COMM_WORLD);
        buffer = new char[buffer_size];
        bzero(buffer, buffer_size);
        pack_counter = 0;
        for (int x = start_x - 1; x < end_x + 1; x++) {
            for (int y = start_y - 1; y < end_y + 1; y++) {
                char life_state_byte = (char)board_read->getPos(x, y);
                MPI::CHAR.Pack(&life_state_byte, 1, buffer, buffer_size, pack_counter, MPI::COMM_WORLD);
            }
        }
        MPI::COMM_WORLD.Send(buffer, pack_counter, MPI::PACKED, rank, 2);
        delete[] buffer;
    } else {
        // send surroundings of board area
        int buffer_size = MPI::CHAR.Pack_size((width + 2) * 2 + (height + 2) * 2, MPI::COMM_WORLD);
        char *buffer = new char[buffer_size];
        bzero(buffer, buffer_size);
        int pack_counter = 0;

        for (int x = start_x - 1; x < end_x + 1; x++) {
            char life_state_byte_up = (char)board_read->getPos(x, start_y - 1);
            char life_state_byte_down = (char)board_read->getPos(x, end_y);
            MPI::CHAR.Pack(&life_state_byte_up, 1, buffer, buffer_size, pack_counter, MPI::COMM_WORLD);
            MPI::CHAR.Pack(&life_state_byte_down, 1, buffer, buffer_size, pack_counter, MPI::COMM_WORLD);
        }

        for (int y = start_y - 1; y < end_y + 1; y++) {
            char life_state_byte_left = (char)board_read->getPos(start_x - 1, y);
            char life_state_byte_right = (char)board_read->getPos(end_x, y);
            MPI::CHAR.Pack(&life_state_byte_left, 1, buffer, buffer_size, pack_counter, MPI::COMM_WORLD);
            MPI::CHAR.Pack(&life_state_byte_right, 1, buffer, buffer_size, pack_counter, MPI::COMM_WORLD);
        }

        MPI::COMM_WORLD.Send(buffer, pack_counter, MPI::PACKED, rank, 2);
        delete[] buffer;
    }
}

void BoardServerMPIAdvanced::barrier() { MPI::COMM_WORLD.Barrier(); }

void BoardServerMPIAdvanced::calculate_area(int rank, int &start_x, int &start_y, int &end_x, int &end_y) {
    // rows are evenly distributed among clients.
    // If it can not be evenly distributed, early clients get a row more than later clients.
    // Example: 100 rows, 7 clients, 0 = 15, 1 = 15, 2 to 6 = 14

    int client_id = rank - 1;
    int clients = MPI::COMM_WORLD.Get_size() - 1;
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
#include "board/BoardServerMPISimple.h"
#include "misc/Log.h"

BoardServerMPISimple::BoardServerMPISimple(Board *board_read, Board *board_write, int timesteps)
    : board_read(board_read), board_write(board_write), timesteps(timesteps) {}

BoardServerMPISimple::~BoardServerMPISimple() {}

void BoardServerMPISimple::start(Stopwatch *stopwatch) {
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

void BoardServerMPISimple::swap_boards() {
    board_read->clear();
    for (int x = 0; x < board_read->getWidth(); x++) {
        for (int y = 0; y < board_read->getHeight(); y++) {
            board_read->setPos(x, y, board_write->getPos(x, y));
        }
    }
    board_write->clear();
}

void BoardServerMPISimple::broadcast_timesteps() {
    MPI::COMM_WORLD.Bcast(&timesteps, sizeof(timesteps), MPI::BYTE, MPI::COMM_WORLD.Get_rank());
}

void BoardServerMPISimple::receive_areas() {
    for (int i = 0; i < MPI::COMM_WORLD.Get_size(); i++) {
        if (i == MPI::COMM_WORLD.Get_rank())
            continue;
        receive_area(i);
    }
}

void BoardServerMPISimple::receive_area(int rank) {
    int start_x, start_y, end_x, end_y;
    calculate_area(rank, start_x, start_y, end_x, end_y);
    for (int x = start_x; x < end_x; x++) {
        for (int y = start_y; y < end_y; y++) {
            // LOG(INFO) << "[SERVER] "
            //           << "x=" << x << ",y=" << y;
            char life_state_byte = 0;
            MPI::COMM_WORLD.Recv(&life_state_byte, sizeof(life_state_byte), MPI::BYTE, rank, 3);
            board_write->setPos(x, y, (life_status_t)life_state_byte);
        }
    }
}

void BoardServerMPISimple::send_areas(bool first_pass) {
    for (int i = 0; i < MPI::COMM_WORLD.Get_size(); i++) {
        if (i == MPI::COMM_WORLD.Get_rank())
            continue;
        send_area(i, first_pass);
    }
}

void BoardServerMPISimple::send_area(int rank, bool first_pass) {
    int start_x, start_y, end_x, end_y;
    calculate_area(rank, start_x, start_y, end_x, end_y);

    if (first_pass) {
        // send board area and surroundings
        MPI::COMM_WORLD.Send(&start_x, sizeof(start_x), MPI::BYTE, rank, 1);
        MPI::COMM_WORLD.Send(&start_y, sizeof(start_y), MPI::BYTE, rank, 1);
        MPI::COMM_WORLD.Send(&end_x, sizeof(end_x), MPI::BYTE, rank, 1);
        MPI::COMM_WORLD.Send(&end_y, sizeof(end_y), MPI::BYTE, rank, 1);
        for (int x = start_x - 1; x < end_x + 1; x++) {
            for (int y = start_y - 1; y < end_y + 1; y++) {
                char life_state_byte = (char)board_read->getPos(x, y);
                MPI::COMM_WORLD.Send(&life_state_byte, sizeof(life_state_byte), MPI::BYTE, rank, 2);
            }
        }
    } else {
        // send surroundings of board area
        for (int x = start_x - 1; x < end_x + 1; x++) {
            char life_state_byte_up = (char)board_read->getPos(x, start_y - 1);
            char life_state_byte_down = (char)board_read->getPos(x, end_y);
            MPI::COMM_WORLD.Send(&life_state_byte_up, sizeof(life_state_byte_up), MPI::BYTE, rank, 2);
            MPI::COMM_WORLD.Send(&life_state_byte_down, sizeof(life_state_byte_down), MPI::BYTE, rank, 2);
        }

        for (int y = start_y - 1; y < end_y + 1; y++) {
            char life_state_byte_left = (char)board_read->getPos(start_x - 1, y);
            char life_state_byte_right = (char)board_read->getPos(end_x, y);
            MPI::COMM_WORLD.Send(&life_state_byte_left, sizeof(life_state_byte_left), MPI::BYTE, rank, 2);
            MPI::COMM_WORLD.Send(&life_state_byte_right, sizeof(life_state_byte_right), MPI::BYTE, rank, 2);
        }
    }
}

void BoardServerMPISimple::barrier() { MPI::COMM_WORLD.Barrier(); }

void BoardServerMPISimple::calculate_area(int rank, int &start_x, int &start_y, int &end_x, int &end_y) {
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
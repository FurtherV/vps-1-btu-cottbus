#include "client/LifeClientMPIAdvanced.h"
#include "misc/Log.h"

LifeClientMPIAdvanced::LifeClientMPIAdvanced(int root_rank) : root_rank(root_rank) {}

LifeClientMPIAdvanced::~LifeClientMPIAdvanced() {
    if (board != nullptr) {
        delete board;
    }
}

void LifeClientMPIAdvanced::start() {
    receive_timesteps();
    LOG(INFO) << "[CLIENT-" << MPI::COMM_WORLD.Get_rank() - 1 << "] "
              << "Receiving initial area";
    receive_area(true);
    while (current_timestep < timesteps) {
        LOG(INFO) << "[CLIENT-" << MPI::COMM_WORLD.Get_rank() - 1 << "] "
                  << "Calculating step";
        board->step();
        barrier();
        LOG(INFO) << "[CLIENT-" << MPI::COMM_WORLD.Get_rank() - 1 << "] "
                  << "Transmitting results";
        send_area();
        barrier();
        LOG(INFO) << "[CLIENT-" << MPI::COMM_WORLD.Get_rank() - 1 << "] "
                  << "Receiving area";
        receive_area(false);
        current_timestep++;
        board->setCurrentStep(current_timestep);
    }
    LOG(INFO) << "[CLIENT-" << MPI::COMM_WORLD.Get_rank() - 1 << "] "
              << "Simulation done";
}

void LifeClientMPIAdvanced::receive_timesteps() { MPI::COMM_WORLD.Bcast(&timesteps, 1, MPI::INT, root_rank); }

void LifeClientMPIAdvanced::send_area() {

    int buffer_size = MPI::CHAR.Pack_size((board->getWidth() - 2) * (board->getHeight() - 2), MPI::COMM_WORLD);
    char *buffer = new char[buffer_size];
    bzero(buffer, buffer_size);
    int pack_counter = 0;

    for (int x = 1; x < board->getWidth() - 1; x++) {
        for (int y = 1; y < board->getHeight() - 1; y++) {
            char life_state_byte = (char)board->getPos(x, y);
            MPI::CHAR.Pack(&life_state_byte, 1, buffer, buffer_size, pack_counter, MPI::COMM_WORLD);
        }
    }

    MPI::COMM_WORLD.Send(buffer, pack_counter, MPI::PACKED, root_rank, 3);
    delete[] buffer;
}

void LifeClientMPIAdvanced::receive_area(bool first_pass) {
    if (first_pass) {
        // receive board area and surroundings

        int buffer_size = MPI::INT.Pack_size(4, MPI::COMM_WORLD);
        char *buffer = new char[buffer_size];
        int unpack_counter = 0;

        MPI::COMM_WORLD.Recv(buffer, buffer_size, MPI::PACKED, root_rank, 1);
        MPI::INT.Unpack(buffer, buffer_size, &start_x, 1, unpack_counter, MPI::COMM_WORLD);
        MPI::INT.Unpack(buffer, buffer_size, &start_y, 1, unpack_counter, MPI::COMM_WORLD);
        MPI::INT.Unpack(buffer, buffer_size, &end_x, 1, unpack_counter, MPI::COMM_WORLD);
        MPI::INT.Unpack(buffer, buffer_size, &end_y, 1, unpack_counter, MPI::COMM_WORLD);

        int width = (end_x - start_x) + 2;
        int height = (end_y - start_y) + 2;
        board = new LocalBoard(width, height);
        board->clear();

        delete[] buffer;
        buffer_size = MPI::CHAR.Pack_size(board->getWidth() * board->getHeight(), MPI::COMM_WORLD);
        buffer = new char[buffer_size];
        unpack_counter = 0;

        MPI::COMM_WORLD.Recv(buffer, buffer_size, MPI::PACKED, root_rank, 2);

        for (int x = 0; x < board->getWidth(); x++) {
            for (int y = 0; y < board->getHeight(); y++) {
                char life_state_byte = 0;
                MPI::CHAR.Unpack(buffer, buffer_size, &life_state_byte, 1, unpack_counter, MPI::COMM_WORLD);
                board->setPos(x, y, (life_status_t)life_state_byte);
            }
        }

        delete[] buffer;
    } else {
        // receive surroundings of board area
        int buffer_size = MPI::CHAR.Pack_size(board->getWidth() * 2 + board->getHeight() * 2, MPI::COMM_WORLD);
        char *buffer = new char[buffer_size];
        bzero(buffer, buffer_size);
        int unpack_counter = 0;

        MPI::COMM_WORLD.Recv(buffer, buffer_size, MPI::PACKED, root_rank, 2);

        for (int x = 0; x < board->getWidth(); x++) {
            char life_state_byte = 0;
            MPI::CHAR.Unpack(buffer, buffer_size, &life_state_byte, 1, unpack_counter, MPI::COMM_WORLD);
            board->setPos(x, 0, (life_status_t)life_state_byte);

            life_state_byte = 0;
            MPI::CHAR.Unpack(buffer, buffer_size, &life_state_byte, 1, unpack_counter, MPI::COMM_WORLD);
            board->setPos(x, board->getHeight() - 1, (life_status_t)life_state_byte);
        }

        for (int y = 0; y < board->getHeight(); y++) {
            char life_state_byte = 0;
            MPI::CHAR.Unpack(buffer, buffer_size, &life_state_byte, 1, unpack_counter, MPI::COMM_WORLD);
            board->setPos(0, y, (life_status_t)life_state_byte);

            life_state_byte = 0;
            MPI::CHAR.Unpack(buffer, buffer_size, &life_state_byte, 1, unpack_counter, MPI::COMM_WORLD);
            board->setPos(board->getWidth() - 1, y, (life_status_t)life_state_byte);
        }

        delete[] buffer;
    }
}

void LifeClientMPIAdvanced::barrier() { MPI::COMM_WORLD.Barrier(); }
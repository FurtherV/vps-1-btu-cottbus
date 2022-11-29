#include "client/LifeClientMPISimple.h"
#include "misc/Log.h"

LifeClientMPISimple::LifeClientMPISimple(int root_rank) : root_rank(root_rank) {}

LifeClientMPISimple::~LifeClientMPISimple() {
    if (board != nullptr) {
        delete board;
    }
}

void LifeClientMPISimple::start() {
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

void LifeClientMPISimple::receive_timesteps() {
    MPI::COMM_WORLD.Bcast(&timesteps, sizeof(timesteps), MPI::BYTE, root_rank);
}

void LifeClientMPISimple::send_area() {
    for (int x = 1; x < board->getWidth() - 1; x++) {
        for (int y = 1; y < board->getHeight() - 1; y++) {
            // LOG(INFO) << "[CLIENT-" << MPI::COMM_WORLD.Get_rank() - 1 << "] "
            //           << "x=" << x << ",y=" << y;
            char life_state_byte = (char)board->getPos(x, y);
            MPI::COMM_WORLD.Send(&life_state_byte, sizeof(life_state_byte), MPI::BYTE, root_rank, 3);
        }
    }
}

void LifeClientMPISimple::receive_area(bool first_pass) {
    if (first_pass) {
        // receive board area and surroundings
        MPI::COMM_WORLD.Recv(&start_x, sizeof(start_x), MPI::BYTE, root_rank, 1);
        MPI::COMM_WORLD.Recv(&start_y, sizeof(start_y), MPI::BYTE, root_rank, 1);
        MPI::COMM_WORLD.Recv(&end_x, sizeof(end_x), MPI::BYTE, root_rank, 1);
        MPI::COMM_WORLD.Recv(&end_y, sizeof(end_y), MPI::BYTE, root_rank, 1);

        int width = (end_x - start_x) + 2;
        int height = (end_y - start_y) + 2;
        board = new LocalBoard(width, height);
        board->clear();

        for (int x = 0; x < board->getWidth(); x++) {
            for (int y = 0; y < board->getHeight(); y++) {
                char life_state_byte = 0;
                MPI::COMM_WORLD.Recv(&life_state_byte, sizeof(life_state_byte), MPI::BYTE, root_rank, 2);
                board->setPos(x, y, (life_status_t)life_state_byte);
            }
        }
    } else {
        // receive surroundings of board area
        for (int x = 0; x < board->getWidth(); x++) {
            char life_state_byte = 0;
            MPI::COMM_WORLD.Recv(&life_state_byte, sizeof(life_state_byte), MPI::BYTE, root_rank, 2);
            board->setPos(x, 0, (life_status_t)life_state_byte);

            life_state_byte = 0;
            MPI::COMM_WORLD.Recv(&life_state_byte, sizeof(life_state_byte), MPI::BYTE, root_rank, 2);
            board->setPos(x, board->getHeight() - 1, (life_status_t)life_state_byte);
        }

        for (int y = 0; y < board->getHeight(); y++) {
            char life_state_byte = 0;
            MPI::COMM_WORLD.Recv(&life_state_byte, sizeof(life_state_byte), MPI::BYTE, root_rank, 2);
            board->setPos(0, y, (life_status_t)life_state_byte);

            life_state_byte = 0;
            MPI::COMM_WORLD.Recv(&life_state_byte, sizeof(life_state_byte), MPI::BYTE, root_rank, 2);
            board->setPos(board->getWidth() - 1, y, (life_status_t)life_state_byte);
        }
    }
}

void LifeClientMPISimple::barrier() { MPI::COMM_WORLD.Barrier(); }
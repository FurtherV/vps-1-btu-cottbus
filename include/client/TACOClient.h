#ifndef TACOCLIENT_H
#define TACOCLIENT_H

#include "board/LocalBoard.h"
#include "taco/tacoMain.h"

class TACOClient {
  public:
    TACOClient(int start_x, int start_y, int end_x, int end_y, taco::ObjectPtr<LocalBoard> server_board)
        : start_x(start_x), start_y(start_y), end_x(end_x), end_y(end_y),
          client_board(end_x - start_x, (end_y - start_y) + 2), server_board(server_board) {}

    ~TACOClient() {}

    bool import_area() {
        // clear local board
        client_board.clear();

        // loop through the assigned area + upper and lower border points and read values from server into our client
        // local board.
        // x and y are server_board positions while local_x and local_y are client board positions.
        for (int y = start_y - 1; y < end_y + 1; y++) {
            for (int x = start_x; x < end_x; x++) {
                int local_x = x - start_x;
                int local_y = y - start_y;
                life_status_t remote_state = server_board->invoke(taco::m2f(&LocalBoard::getPos, x, y));
                client_board.setPos(local_x, local_y, remote_state);
            }
        }
        return true;
    }

    bool step() {
        client_board.step();
        return true;
    }

    bool export_area() {
        // loop through the assigned area and write values to server from our client board.
        // x and y are server_board positions while local_x and local_y are client board positions.
        for (int y = start_y; y < end_y; y++) {
            for (int x = start_x; x < end_x; x++) {
                int local_x = x - start_x;
                int local_y = y - start_y;
                life_status_t local_state = client_board.getPos(local_x, local_y);
                server_board->call(taco::m2f(&LocalBoard::setPos, x, y, local_state));
            }
        }

        return true;
    }

  private:
    int start_x, start_y, end_x, end_y;
    LocalBoard client_board;
    taco::ObjectPtr<LocalBoard> server_board;
};

#endif

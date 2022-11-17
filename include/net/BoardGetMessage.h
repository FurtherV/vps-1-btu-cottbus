#ifndef BOARDGETMESSAGE_H
#define BOARDGETMESSAGE_H

#include "board/Board.h"
#include "net/Message.h"

class BoardGetMessage : public Message {
  private:
    BoardGetMessage() = delete;
    BoardGetMessage(unsigned int sequence_number) : Message(message_type_t::board_get, sequence_number) {}

  public:
    /**
     * @brief Helper function to create a 'board get' request message.
     * @return unmanaged pointer to the created message.
     */
    static BoardGetMessage *createRequest(unsigned int sequence_number, int x, int y) {
        BoardGetMessage *message = new BoardGetMessage(sequence_number);
        message->pos_x = x;
        message->pos_y = y;
        message->toRequest();
        return message;
    };

    /**
     * @brief Helper function to create a 'board get' reply message.
     * @return unmanaged pointer to the created message.
     */
    static BoardGetMessage *createReply(unsigned int sequence_number, int x, int y, Board *board) {
        BoardGetMessage *message = new BoardGetMessage(sequence_number);
        message->pos_x = x;
        message->pos_y = y;
        message->state = board->getPos(x, y);
        message->toReply();
        return message;
    };

    int pos_x, pos_y = 0;
    life_status_t state = life_status_t::invalid;
};

#endif // BOARDGETMESSAGE_H
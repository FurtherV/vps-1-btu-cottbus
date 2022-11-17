#ifndef BOARDSETMESSAGE_H
#define BOARDSETMESSAGE_H

#include "board/Board.h"
#include "net/Message.h"

class BoardSetMessage : public Message {
  private:
    BoardSetMessage() = delete;
    BoardSetMessage(unsigned int sequence_number) : Message(message_type_t::board_set, sequence_number) {}

  public:
    /**
     * @brief Helper function to create a 'board get' request message.
     * @return unmanaged pointer to the created message.
     */
    static BoardSetMessage *createRequest(unsigned int sequence_number, int x, int y, life_status_t state) {
        BoardSetMessage *message = new BoardSetMessage(sequence_number);
        message->pos_x = x;
        message->pos_y = y;
        message->state = state;
        message->toRequest();
        return message;
    };

    /**
     * @brief Helper function to create a 'board get' reply message.
     * @return unmanaged pointer to the created message.
     */
    static BoardSetMessage *createReply(unsigned int sequence_number) {
        BoardSetMessage *message = new BoardSetMessage(sequence_number);
        message->confirmed = true;
        message->toReply();
        return message;
    };

    int pos_x, pos_y = 0;
    life_status_t state = life_status_t::invalid;
    bool confirmed = false;
};

#endif // BOARDSETMESSAGE_H
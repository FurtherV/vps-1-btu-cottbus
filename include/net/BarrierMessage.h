#ifndef BARRIERMESSAGE_H
#define BARRIERMESSAGE_H

#include "net/Message.h"

class BarrierMessage : public Message {
  private:
    BarrierMessage() = delete;
    BarrierMessage(unsigned int sequence_number) : Message(message_type_t::barrier, sequence_number) {}

  public:
    /**
     * @brief Helper function to create a 'board get' request message.
     * @return unmanaged pointer to the created message.
     */
    static BarrierMessage *createRequest(unsigned int sequence_number, int client_id, int finished_timestep) {
        BarrierMessage *message = new BarrierMessage(sequence_number);
        message->client_id = client_id;
        message->finished_timestep = finished_timestep;
        message->toRequest();
        return message;
    };

    /**
     * @brief Helper function to create a 'board get' reply message.
     * @return unmanaged pointer to the created message.
     */
    static BarrierMessage *createReply(unsigned int sequence_number, int client_id) {
        BarrierMessage *message = new BarrierMessage(sequence_number);
        message->client_id = client_id;
        message->continueNext = true;
        message->toReply();
        return message;
    };

    int client_id = -1;
    int finished_timestep = -1;
    bool continueNext = false;
};

#endif
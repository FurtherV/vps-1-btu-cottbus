#ifndef LOGONMESSAGE_H
#define LOGONMESSAGE_H

#include "net/Message.h"

class LogonMessage : public Message {
  private:
    LogonMessage() = delete;
    LogonMessage(unsigned int sequence_number) : Message(message_type_t::logon, sequence_number) {}

  public:
    /**
     * @brief Helper function to create a logon request message.
     * @param sequence_number sequence number of the message
     * @return unmanaged pointer to the created message.
     */
    static LogonMessage *createRequest(unsigned int sequence_number) {
        LogonMessage *msg = new LogonMessage(sequence_number);
        msg->toRequest();
        return msg;
    };

    /**
     * @brief Helper function to create a logon reply message.
     * @param sequence_number sequence number, should match a previously received request.
     * @param start_x x position on a board
     * @param start_y y position on a board
     * @param end_x x position on a board
     * @param end_y y position on a board
     * @param timesteps simulation cycles the client has to compute
     * @return unmanaged pointer to the created message.
     */
    static LogonMessage *createReply(unsigned int sequence_number, int client_id, int start_x, int start_y, int end_x,
                                     int end_y, int timesteps) {
        LogonMessage *msg = new LogonMessage(sequence_number);
        msg->client_id = client_id;
        msg->start_x = start_x;
        msg->start_y = start_y;
        msg->end_x = end_x;
        msg->end_y = end_y;
        msg->timesteps = timesteps;
        msg->toReply();
        return msg;
    };

    int client_id;
    int start_x;
    int start_y;
    int end_x;
    int end_y;
    int timesteps;
};

#endif // LOGONMESSAGE_H
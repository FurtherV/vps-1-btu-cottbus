#ifndef MESSAGE_H
#define MESSAGE_H

enum class message_type_t { invalid, logon, board_set, board_get, barrier };
enum class message_mode_t { invalid, request, reply };

class Message {
  public:
    Message() = delete;
    message_type_t getType() { return type; }
    unsigned int getSequenceNumber() { return sequence_number; }
    void toRequest() { mode = message_mode_t::request; }
    void toReply() { mode = message_mode_t::reply; }

  protected:
    Message(message_type_t type, unsigned int sequence_number) : type(type), sequence_number(sequence_number) {}

  private:
    message_type_t type = message_type_t::invalid;
    message_mode_t mode = message_mode_t::invalid;
    unsigned int sequence_number = 0;
};

#endif
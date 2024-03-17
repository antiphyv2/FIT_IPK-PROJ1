#include "socket.hpp"

typedef enum {
    AUTH,
    JOIN,
    ERR,
    BYE,
    MSG,
    REPLY,
    NOT_REPLY,
    NOTHING,

} msg_types;

class TCPMessage {

    private:
        msg_types type;
        std::string message_to_process;
        char buffer[1500];
        //char* buffer;

    public:
        TCPMessage(std::string input_msg);
        void process_input_msg();
        void print_message();
        void fill_output_buffer(std::string msg_part);
        const char* get_buffer();
        size_t get_buffer_size();
        void print_buffer();
        void add_line_ending();
};
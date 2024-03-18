#include "socket.hpp"

typedef enum {
    AUTH,
    JOIN,
    ERR,
    BYE,
    MSG,
    REPLY,
    NOT_REPLY,
    RENAME,
    HELP,
    USER_CMD,
} msg_types;

class TCPMessage {

    private:
        msg_types type;
        bool ready_to_send;
        std::string display_name;
        std::string message;
        char buffer[1500];

    public:
        TCPMessage(std::string input_msg, std::string dname, msg_types msg_type);

        void copy_msg_to_buffer();
        bool validate_msg_param(std::string parameter, std::string pattern);
        bool is_ready_to_send();
        void print_message();
        void add_to_buffer(std::string msg_part);
        char* get_buffer();
        void clear_buffer();
        size_t get_buffer_size();
        void print_buffer();
        std::string get_display_name();
        void set_display_name(std::string name);
        msg_types get_msg_type();
        void add_line_ending();
};
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
    NOTHING,
} msg_types;

class TCPMessage {

    private:
        msg_types type;
        bool ready_to_send;
        std::string display_name;
        std::string local_msg;
        std::string msg_from_server;
        char buffer[1500];

    public:
        TCPMessage(std::string input_msg, std::string dname);

        void process_local_msg();
        void process_msg_from_server();
        bool validate_msg_param(std::string parameter, std::string pattern);
        bool is_ready_to_send();


        void print_local_message();
        void print_msg_from_server();

        void add_to_buffer(std::string msg_part);
        const char* get_buffer();
        size_t get_buffer_size();
        void print_buffer();
        std::string get_display_name();
        void set_display_name(std::string name);
        msg_types get_msg_type();

        void add_line_ending();
};
#ifndef MESSAGES_HPP
#define MESSAGES_HPP
#include <iostream>
#include <vector>
#include <sstream>
#include <string>
#include <cstring>

typedef enum {
    AUTH,
    JOIN,
    ERR,
    BYE,
    MSG,
    REPLY_OK,
    REPLY_NOK,
    RENAME,
    HELP,
    USER_CMD,
    TO_BE_DECIDED,
} msg_types;

class TCPMessage {

    private:
        msg_types type;
        bool ready_to_send;
        std::string display_name;
        std::string message;
        char buffer[1500];

    public:
        TCPMessage(std::string input_msg, msg_types msg_type);

        void copy_msg_to_buffer();
        void process_recv_msg();
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
        void set_msg_type(msg_types msg_type);
        void add_line_ending();
};
#endif
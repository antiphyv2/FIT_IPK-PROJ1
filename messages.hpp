#ifndef MESSAGES_HPP
#define MESSAGES_HPP
#include <iostream>
#include <vector>
#include <sstream>
#include <string>
#include <cstring>
#include <regex>

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

class NetworkMessage{

    protected:
        msg_types type;
        bool ready_to_send;
        std::string display_name;
        std::string message;
        char buffer[1500];
        
        NetworkMessage(std::string input_msg, msg_types msg_type);

    public:
        virtual void process_outgoing_msg() = 0;
        virtual void process_inbound_msg(size_t bytes_rx) = 0;
        virtual void add_to_buffer(std::string msg_part) = 0;
        bool is_ready_to_send();
        void print_message();
        char* get_buffer();
        void clear_buffer();
        size_t get_buffer_size();
        void print_buffer();
        std::string get_display_name();
        void set_display_name(std::string name);
        msg_types get_msg_type();
        void set_msg_type(msg_types msg_type);

        //NetworkMessage(std::string input_msg, msg_types msg_type);
        virtual ~NetworkMessage() {}
};

class TCPMessage : public NetworkMessage{

    public:
        TCPMessage(std::string input_msg, msg_types msg_type);

        void process_outgoing_msg() override;
        void process_inbound_msg(size_t bytes_rx) override;
        bool validate_msg_param(std::string parameter, std::string pattern);
        void add_to_buffer(std::string msg_part) override;
        void add_line_ending();
        void remove_line_ending(std::string& message);
};
#endif
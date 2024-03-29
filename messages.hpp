#ifndef MESSAGES_HPP
#define MESSAGES_HPP
#include <iostream>
#include <vector>
#include <sstream>
#include <string>
#include <cstring>
#include <regex>
#include <iomanip>

#define BUFFER_SIZE 1500
#define UDP_CONFIRM 0x00
#define UDP_REPLY 0x01
#define UDP_AUTH 0x02
#define UDP_JOIN 0x03
#define UDP_MSG 0x04
#define UDP_ERR 0xFE
#define UDP_BYE 0xFF

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
    CONFIRM,
} msg_types;

class NetworkMessage{

    protected:
        msg_types type;
        bool ready_to_send;
        std::string display_name;
        std::string message;
        char buffer[BUFFER_SIZE];
        
        NetworkMessage(std::string input_msg, msg_types msg_type);

    public:
        virtual void process_outgoing_msg() = 0;
        virtual void process_inbound_msg(size_t bytes_rx) = 0;
        bool is_ready_to_send();
        void print_message();
        void clear_buffer();
        void print_buffer();
        virtual void* get_buffer() = 0;
        virtual size_t get_buffer_size() = 0;
        std::string get_display_name();
        void set_display_name(std::string name);
        msg_types get_msg_type();
        void set_msg_type(msg_types msg_type);
        bool validate_msg_param(std::string parameter, std::string pattern);
        void check_user_message(std::vector<std::string>& message_parts);

        //NetworkMessage(std::string input_msg, msg_types msg_type);
        virtual ~NetworkMessage() {}
};

class TCPMessage : public NetworkMessage{

    public:
        TCPMessage(std::string input_msg, msg_types msg_type) : NetworkMessage(input_msg, msg_type){}

        void process_outgoing_msg() override;
        void process_inbound_msg(size_t bytes_rx) override;
        void add_to_buffer(std::string msg_part);
        void* get_buffer();
        size_t get_buffer_size();
        void add_line_ending();
        void remove_line_ending(std::string& message);
};

class UDPMessage : public NetworkMessage{

    private:
        bool waiting_for_confirm;
        std::vector<uint8_t> udp_buffer;
        uint16_t message_id;
    public:
        UDPMessage(std::string input_msg, msg_types msg_type, uint16_t msg_id) : NetworkMessage(input_msg, msg_type), message_id(msg_id){}
        void process_outgoing_msg() override;
        void process_inbound_msg(size_t bytes_rx) override;
        void* get_buffer();
        size_t get_buffer_size();
};
#endif
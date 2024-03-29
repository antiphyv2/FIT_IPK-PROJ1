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
        msg_types type; //MSG type
        bool ready_to_send; //MSG is ready to send to the server
        std::string display_name; //Actual display name
        std::string message; //Message from user or reply msg to be printed
        char buffer[BUFFER_SIZE]; //Buffer for incoming messages
        
        NetworkMessage(std::string input_msg, msg_types msg_type);

    public:
        /**
         * @brief Virtual function for parsing output message
         * 
         */
        virtual void process_outgoing_msg() = 0;

        /**
         * @brief Virtual function for parsing inbound message
         * 
         * @param bytes_rx Bytes received
         */
        virtual void process_inbound_msg(size_t bytes_rx) = 0;

        /**
         * @brief Message is ready to be send to sever
         * 
         * @return true if message is ready
         * @return false if message is local only
         */
        bool is_ready_to_send();

        /**
         * @brief Prints message from user or reply msg
         * 
         */
        void print_message();

        /**
         * @brief Clears buffer
         * 
         */
        void clear_buffer();

        /**
         * @brief Prints buffer
         * 
         */
        void print_buffer();

        /**
         * @brief Virtual function to get a pointer for the buffer
         * 
         * @return void* pointer to the buffer depending on derived class
         */
        virtual void* get_buffer() = 0;

        /**
         * @brief Virtual function to get buffer size
         * 
         * @return size_t buffer size
         */
        virtual size_t get_buffer_size() = 0;

        /**
         * @brief Gets the display name
         * 
         * @return std::string display name
         */
        std::string get_display_name();

        /**
         * @brief Sets the display name
         * 
         * @param name display name
         */
        void set_display_name(std::string name);

        /**
         * @brief Gets the message type
         * 
         * @return msg_types message type
         */
        msg_types get_msg_type();

        /**
         * @brief Sets the message type
         * 
         * @param msg_type to be set
         */
        void set_msg_type(msg_types msg_type);

        /**
         * @brief Validates parameter according to IPK grammar rules with specific pattern
         * 
         * @param parameter Message to be checked
         * @param pattern Pattern which message will be checked with
         * @return true if validation succesful
         * @return false if validation unsuccesful
         */
        bool validate_msg_param(std::string parameter, std::string pattern);

        /**
         * @brief Checks if message is from user and split message to parts accordingly
         * 
         * @param message_parts vector of message parts obtained from user
         */
        void check_user_message(std::vector<std::string>& message_parts);

        //NetworkMessage(std::string input_msg, msg_types msg_type);
        virtual ~NetworkMessage() {}
};

class TCPMessage : public NetworkMessage{

    public:
        TCPMessage(std::string input_msg, msg_types msg_type) : NetworkMessage(input_msg, msg_type){}

        void process_outgoing_msg() override;
        void process_inbound_msg(size_t bytes_rx) override;

        /**
         * @brief Adds message part to buffer as a preparation for sending message
         * 
         * @param msg_part to be added to buffer
         */
        void add_to_buffer(std::string msg_part);
        void* get_buffer();
        size_t get_buffer_size();
        void add_line_ending();
        /**
         * @brief Removes \r\n from TCP message
         * 
         * @param message where line ending should be removed
         */
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
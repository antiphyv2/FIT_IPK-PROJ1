#ifndef CLIENTS_HPP
#define CLIENTS_HPP

#include "main.hpp"
#include "socket.hpp"


typedef struct CL_INFO{
    std::string dname; //User display name
    fsm_states client_state = START_STATE; //Current mat state
    bool reply_msg_sent = false; //Message will require REPLY from server
    int msg_counter = 0; //MSG counter for UDP
} client_info;

class NetworkClient{

    protected:
        connection_info* conn_info; //Structure with connection info retreived from CLI
        client_info cl_info; //Structure with current client information
        ClientSocket* socket; //Socket object
        struct addrinfo* dns_results; //Structure for DNS retrieval
        
        NetworkClient(connection_info* info);
    public:
        /**
         * @brief Virtual method to receive message in buffer
         * 
         * @param msg containing buffer in which incoming mesage will be stored
         * @return size_t bytes received
         */
        virtual int accept_msg(NetworkMessage& msg) = 0;

        /**
         * @brief Virtual method to send message over socket networking
         * 
         * @param msg to be send over socket
         */
        virtual void send_msg(NetworkMessage& msg) = 0;

        /**
         * @brief Returns pointer to structure with CLI provided arguments
         * 
         * @return connection_info* pointer to the structure
         */
        connection_info* get_arg_info();

        /**
         * @brief Gets pointer to the network socket 
         * 
         * @return ClientSocket* pointer to the socket
         */
        ClientSocket* get_socket();

        /**
         * @brief Returns pointer to structure with client information
         * 
         * @return client_info* pointer to the structure
         */
        client_info* get_cl_info();

        /**
         * @brief Translates domain name to IP address
         * 
         */
        void dns_lookup();

        /**
         * @brief Returns pointer to structure with dns information
         * 
         * @return struct addrinfo* pointer to the structure
         */
        struct addrinfo* get_dns_info();
        virtual ~NetworkClient();
    
};

class TCPClient : public NetworkClient{

    public:
        TCPClient(connection_info* info) : NetworkClient(info){}
        /**
         * @brief Main function for TCP communication
         * 
         */
        void start_tcp_chat();

        /**
         * @brief Connects to server with parameters in dns structure
         * 
         */
        void establish_connection();

        void send_msg(NetworkMessage& msg) override;
        int accept_msg(NetworkMessage& msg) override;
        ~TCPClient();
};

class UDPClient : public NetworkClient{
    private:
        struct sockaddr_in server_addr; //Structure for receiving server info
        uint16_t server_port; //Server port needed in case of incoming reply from other port
        bool confirm_msg_sent; //Client will wait for confirm message
        bool change_server_port; //Indicates if server port has been changed 
        int retry_count; //Number of already happened retry counts
        bool timeout_happened; //Timeout has ocured on socket
        std::vector<uint16_t> confirm_id_vector; //Vector of ids to confirm
        std::vector<uint16_t> reply_id_vector; //Vector of ids to reply
        std::vector<uint16_t> seen_ids; //Vector of seen ids
    public:
        UDPClient(connection_info* info);

        /**
         * @brief Main function for UDP communication
         * 
         */
        void start_udp_chat();

        void send_msg(NetworkMessage& msg) override;
        int accept_msg(NetworkMessage& msg) override;

        /**
         * @brief Sends confirm message and exits program if needed
         * 
         * @param msg Confirm message to be send
         * @param exit True if program should exit
         */
        void send_confim_exit(UDPMessage msg, bool exit);

        /**
         * @brief Gets a pointer to a vector of seen ids
         * 
         * @return std::vector<uint16_t>* returns pointer to a vector of seen ids
         */
        std::vector<uint16_t>* get_seen_ids();

        /**
         * @brief Gets retry count number
         * 
         * @return int returns retry count
         */
        int get_retry_count();
        
        /**
         * @brief Sends error mesage waits for confirm and exits
         * 
         * @param confirm_id Confirm_id to be validated
         * @param reply_id Reply_id to be validated
         */
        void send_error_exit(int* confirm_id, int* reply_id);

        /**
         * @brief Waits for confirm and send message back accoreding to number of retries
         * 
         * @param confirm_id Confirm_id to be validated
         * @param reply_id Reply_id to be validated
         * @param outgoing_msg Message to be sent if confirm timeout happened
         * @param exit_program true if program should end
         */
        void handle_timeout(int* confirm_id, int* reply_id, UDPMessage& outgoing_msg, bool exit_program);

        /**
         * @brief Handles message and validates if message is in correct state
         * 
         * @param skip_message If message should be skipper
         * @param inbound_msg Message to be processed
         * @param confirm_id Id to be validated
         * @param reply_id Reply_id to be validated
         * @param bytes_rx Bytes received
         */
        void fsm_logic_handler(bool* skip_message, UDPMessage& inbound_msg, int* confirm_id, int* reply_id, int bytes_rx);
        ~UDPClient();
};


#endif
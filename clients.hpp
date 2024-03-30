#ifndef CLIENTS_HPP
#define CLIENTS_HPP

#include "main.hpp"
#include "socket.hpp"


typedef struct CL_INFO{
    std::string dname; //User display name
    fsm_states client_state = START_STATE; //Current mat state
    bool reply_msg_sent = false; //Message will require REPLY from server
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
         * @brief Sends message over socket networking
         * 
         * @param msg to be send over socket
         */
        void send_msg(NetworkMessage& msg);

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
         * @brief Connects to server with parameters in dns structure
         * 
         */
        void establish_connection();

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
        void start_tcp_chat();
        int accept_msg(NetworkMessage& msg) override;
        ~TCPClient();
};

class UDPClient : public NetworkClient{
    private:
        struct sockaddr_in server_addr;
    public:
        UDPClient(connection_info* info);
        void start_udp_chat();

        int accept_msg(NetworkMessage& msg) override;
        ~UDPClient();
};


#endif
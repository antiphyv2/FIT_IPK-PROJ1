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
        virtual size_t accept_msg(NetworkMessage& msg) = 0;
        void send_msg(NetworkMessage& msg);
        connection_info* get_arg_info();
        ClientSocket* get_socket();
        client_info* get_cl_info();
        void dns_lookup();
        void establish_connection();
        struct addrinfo* get_dns_info();
        virtual ~NetworkClient();
    
};

class TCPClient : public NetworkClient{

    public:
        TCPClient(connection_info* info) : NetworkClient(info){}
        void start_tcp_chat();
        size_t accept_msg(NetworkMessage& msg) override;
        ~TCPClient();
};

class UDPClient : public NetworkClient{
    private:
        struct sockaddr_in server_addr;
    public:
        UDPClient(connection_info* info);
        void start_udp_chat();

        size_t accept_msg(NetworkMessage& msg) override;
        ~UDPClient();
};


#endif
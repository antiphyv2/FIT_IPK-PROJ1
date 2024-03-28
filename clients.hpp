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
        connection_info* conn_info;
        client_info cl_info;
        ClientSocket* socket;
        struct addrinfo* dns_results; //Structure for DNS retrieval
        
        NetworkClient(connection_info* info);
    public:
        
        connection_info* get_arg_info();
        ClientSocket* get_socket();
        void dns_lookup();
        void establish_connection();
        void send_msg(NetworkMessage& msg);
        size_t accept_msg(NetworkMessage* msg);
        struct addrinfo* get_dns_info();
        ~NetworkClient();
    
};

class TCPClient : public NetworkClient{

    public:
        TCPClient(connection_info* info) : NetworkClient(info){}
        void start_tcp_chat();
        //~TCPClient();
};


#endif
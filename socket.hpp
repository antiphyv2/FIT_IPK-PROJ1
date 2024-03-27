#ifndef SOCKET_HPP
#define SOCKET_HPP

#include "proj1.hpp"
#include "messages.hpp"


typedef struct CL_INFO{
    std::string dname; //User display name
    fsm_states client_state = START_STATE; //Current mat state
    std::queue<TCPMessage> msgQ; //MSG queue for storing messages until processing is enabed
    bool reply_msg_sent = false; //Message will require REPLY from server
} client_info;

class ClientSocket{
    public:

    private:
        int socket_fd; //Socket FD
        int type; //TCP or UDP
        struct addrinfo* dns_results; //Structure for DNS retrieval
        connection_info* info; //CLI arguments

    public:
        ClientSocket(connection_info* parsed_info);
        void create_socket();
        void cleanup();

        int get_socket_fd();
        int get_socket_type();
        struct addrinfo* get_dns_info();
        connection_info* get_arg_info();

        void dns_lookup();
        void establish_connection();
        void send_msg(TCPMessage msg);
        size_t accept_msg(TCPMessage* msg);
        void print_args();
        void start_tcp_chat();
};
#endif
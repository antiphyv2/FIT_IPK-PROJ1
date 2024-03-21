#ifndef SOCKET_HPP
#define SOCKET_HPP

#include "proj1.hpp"
#include "messages.hpp"

typedef struct CL_INFO{
    std::string dname;
    fsm_states client_state = START_STATE;
    std::queue<TCPMessage> msgQ;
    //bool awaiting_reply = false;
    //bool auth_sent = false;
    bool reply_msg_sent = false;
} client_info;
class ClientSocket{
    public:

    private:
        int socket_fd; //Socket FD
        int epoll_fd;
        int type; //TCP or UDP
        struct addrinfo* dns_results; //Structure for DNS retrieval
        connection_info* info; //CLI arguments

    public:
        ClientSocket(int sock_type, connection_info* parsed_info);
        void create_socket();
        void cleanup();

        int get_socket_fd();
        int get_epoll_fd();
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
#ifndef SOCKET_HPP
#define SOCKET_HPP

#include "main.hpp"
#include "messages.hpp"
#include "arg_parser.hpp"

class ClientSocket{

    private:
        int socket_fd; //Socket FD
        int type; //TCP or UDP

    public:
        ClientSocket(int protocol_type);
        ~ClientSocket();
        void create_socket();

        int get_socket_fd();
        int get_socket_type();
};
#endif
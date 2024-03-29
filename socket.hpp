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

        /**
         * @brief Creates socket 
         * 
         */
        void create_socket();

        /**
         * @brief Socket fd getter
         * 
         * @return file descriptor of the socket
         */
        int get_socket_fd();
        /**
         * @brief Socket type getter
         * 
         * @return socket type
         */
        int get_socket_type();
};
#endif
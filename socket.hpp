#ifndef SOCKET_HPP
#define SOCKET_HPP

#include "main.hpp"
#include "messages.hpp"
#include "arg_parser.hpp"

class ClientSocket{

    private:
        int socket_fd; //Socket FD
        int type; //TCP or UDP
        struct timeval tv; //socket timeout

    public:
        ClientSocket(int protocol_type);
        ~ClientSocket();

        /**
         * @brief Creates network tcp/udp socket 
         * 
         * @param info pointer to user provided arguments
         */
        void create_socket(connection_info* info);

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

        /**
         * @brief Gets a pointer to the socket timeval struct
         * 
         * @return struct timeval* pointer to the struct
         */
        struct timeval* get_socket_tv();
};
#endif
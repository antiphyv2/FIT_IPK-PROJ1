#include "socket.hpp"

ClientSocket::ClientSocket(int protocol_type){
    socket_fd = -1;
    type = protocol_type;
    tv = {.tv_sec = 0, .tv_usec = 0};
}

ClientSocket::~ClientSocket(){
    int ret_val;
    if(socket_fd != -1){
        if((ret_val = close(socket_fd)) == -1){
            std::cerr << "ERR: CLOSING SOCKET." << std::endl;
            exit(EXIT_FAILURE);
        }
    }
}

void ClientSocket::create_socket(connection_info* info){
    if((socket_fd = socket(AF_INET, type, 0)) == -1){
        std::cerr << "ERR: CREATING SOCKET." << std::endl;
        exit(EXIT_FAILURE);
    }

    if(type == SOCK_DGRAM){
        tv.tv_usec = info->udp_timeout * 1000;
        int ret_val;
        if((ret_val = setsockopt(socket_fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv))) == -1){
            std::cerr << "ERR: SETTING SOCKET TIMEOUT." << std::endl;
            exit(EXIT_FAILURE);
        }   
    }
}

int ClientSocket::get_socket_type(){
    return type;
}

int ClientSocket::get_socket_fd(){
    return socket_fd;
}

struct timeval* ClientSocket::get_socket_tv(){
    return &tv;
}
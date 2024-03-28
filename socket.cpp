#include "socket.hpp"

ClientSocket::ClientSocket(int protocol_type){
    socket_fd = -1;
    type = protocol_type;
    
}

ClientSocket::~ClientSocket(){
    int ret_val;
    if(socket_fd != -1){
        if((ret_val = close(socket_fd)) == -1){
            std::cerr << "ERR: CLOSING SOCKET." << std::endl;
            exit(EXIT_FAILURE);
        }
    }
    std::cout << "SOCKETS SUCCESFULLY CLOSED." << std::endl;
}

void ClientSocket::create_socket(){
    if((socket_fd = socket(AF_INET, type, 0)) == -1){
        std::cerr << "ERR: CREATING SOCKET." << std::endl;
        cleanup();
        exit(EXIT_FAILURE);
    }

    std::cout << "SOCKET SUCCESFULLY CREATED." << std::endl;
}

void ClientSocket::cleanup(){

    int ret_val;
    if(socket_fd != -1){
        if((ret_val = close(socket_fd)) == -1){
            std::cerr << "ERR: CLOSING SOCKET." << std::endl;
            exit(EXIT_FAILURE);
        }
    }
    std::cout << "SOCKETS SUCCESFULLY CLOSED." << std::endl;
}

int ClientSocket::get_socket_type(){
    return type;
}

int ClientSocket::get_socket_fd(){
    return socket_fd;
}

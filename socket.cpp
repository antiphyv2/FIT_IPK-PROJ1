#include "socket.hpp"

ClientSocket::ClientSocket(int sock_type){
    if((socket_fd = socket(AF_INET, sock_type, 0)) == -1){
        std::cerr << "ERR: CREATING SOCKET." << std::endl;
        cleanup();
        exit(EXIT_FAILURE);
    }
    std::cout << "SOCKET SUCCESFULLY CREATED." << std::endl;

    type = sock_type;
    dns_results = nullptr;
    info = nullptr;
}

void ClientSocket::cleanup(){
    if(dns_results != nullptr){
        freeaddrinfo(dns_results);
    }
    if(info != nullptr){
        delete info;
    }
    int socket_close;
    if((socket_close = close(socket_fd)) == -1){
        std::cerr << "ERR: CLOSING SOCKET." << std::endl;
        exit(EXIT_FAILURE);
    }

    std::cout << "SOCKET SUCCESFULLY CLOSED." << std::endl;
}

int ClientSocket::get_socket_type(){
    return type;
}

int ClientSocket::get_socket_fd(){
    return socket_fd;
}

connection_info* ClientSocket::get_arg_info(){
    return info;
}

struct addrinfo* ClientSocket::get_dns_info(){
    return dns_results;
}

void ClientSocket::set_arg_info(connection_info* parsed_info){
    info = parsed_info;
}

void ClientSocket::print_args(){
    std::cout << "Protocol:" << info->protocol << std::endl;
    std::cout << "IP/Hostname:" << info->ip_hostname << std::endl;
    std::cout << "Port:" << info->port << std::endl;
    std::cout << "UDP timeout:" << info->udp_timeout << std::endl;
    std::cout << "Max UDP retransmission:" << info->max_udp_retransmission << std::endl;
}

void ClientSocket::dns_lookup(){
    struct addrinfo hints = {0};
    hints.ai_family = AF_INET;
    hints.ai_socktype = type;
    hints.ai_protocol = 0;

    int retreived_info;
    if ((retreived_info = getaddrinfo(info->ip_hostname.c_str(), info->port.c_str(), &hints, &dns_results)) != 0){
        std::cout << "ERR: Could not resolve hostname." << std::endl;
        cleanup();
        exit(EXIT_FAILURE);
    }
}

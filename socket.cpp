#include "socket.hpp"

ClientSocket::ClientSocket(int sock_type,  connection_info* parsed_info){

    socket_fd == -1;
    epoll_fd = -1;
    type = sock_type;
    dns_results = nullptr;
    info = parsed_info;
}

void ClientSocket::create_socket(){
        if((socket_fd = socket(AF_INET, type, 0)) == -1){
        std::cerr << "ERR: CREATING SOCKET." << std::endl;
        cleanup();
        exit(EXIT_FAILURE);
    }

    if((epoll_fd = epoll_create1(0)) == -1){
        std::cerr << "ERR: CREATING EPOLL." << std::endl;
        cleanup();
        exit(EXIT_FAILURE);
    }

    std::cout << "SOCKET SUCCESFULLY CREATED." << std::endl;
}

void ClientSocket::cleanup(){
    if(dns_results != nullptr){
        freeaddrinfo(dns_results);
    }
    if(info != nullptr){
        delete info;
    }

    int ret_val;
    if(socket_fd != -1){
        if((ret_val = close(socket_fd)) == -1){
            std::cerr << "ERR: CLOSING SOCKET." << std::endl;
            exit(EXIT_FAILURE);
        }
    }
    if(epoll_fd != -1){
        if((ret_val= close(epoll_fd)) == -1){
            std::cerr << "ERR: CLOSING EPOLL SOCKET." << std::endl;
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

connection_info* ClientSocket::get_arg_info(){
    return info;
}

struct addrinfo* ClientSocket::get_dns_info(){
    return dns_results;
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

void ClientSocket::establish_connection(){
    int ret_val;
    if((ret_val = connect(socket_fd, dns_results->ai_addr, dns_results->ai_addrlen)) != 0){
        std::cout << "ERR: Could not connect to the server." << std::endl;
        cleanup();
        exit(EXIT_FAILURE);
    }
}

void ClientSocket::send_msg(TCPMessage msg){
    ssize_t bytes_sent = send(socket_fd, msg.get_buffer(), msg.get_buffer_size(), 0);
    if (bytes_sent == -1) {
        std::cerr << "ERR: Message could not be send to server." << std::endl;
    } else {
        std::cout << "Message sent to server." << std::endl;
    }
}

void ClientSocket::start_tcp_chat(){
    create_socket();
    dns_lookup();
    establish_connection();

    fsm_states client_state = START_STATE;
    std::string dname = "";

    if(get_socket_type() == SOCK_STREAM){

        while(true){
            std::string message;
            if(!std::getline(std::cin, message)){
                TCPMessage bye_msg("BYE", BYE);
                bye_msg.proces_outgoing_msg();
                send_msg(bye_msg);
                break;
            }

            if(message.empty()){
                continue;
            }

            TCPMessage outgoing_msg(message, USER_CMD);
            outgoing_msg.set_display_name(dname);
            outgoing_msg.proces_outgoing_msg();
            
            //Set username or change in case of rename command
            if(outgoing_msg.get_msg_type() == AUTH || outgoing_msg.get_msg_type() == RENAME){
                dname = outgoing_msg.get_display_name();
            }

            if(outgoing_msg.is_ready_to_send()){
                outgoing_msg.print_buffer();
                send_msg(outgoing_msg);
            }

            // if (std::cin.eof()) {


            //     // if(client_state != START_STATE){
            //     //     //send msg to server
            //     // }
            //     break;
            // }
        }
    }   
}
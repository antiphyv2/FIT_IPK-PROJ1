#include "socket.hpp"

ClientSocket::ClientSocket(connection_info* parsed_info){

    socket_fd = -1;
    type = parsed_info->sock_type;
    dns_results = nullptr;
    info = parsed_info;
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
    //std::cout << "Protocol:" << info->protocol << std::endl;
    std::cout << "IP/Hostname:" << info->ip_hostname << std::endl;
    std::cout << "Port:" << info->port << std::endl;
    std::cout << "UDP timeout:" << info->udp_timeout << std::endl;
    std::cout << "Max UDP retransmission:" << info->max_udp_retransmission << std::endl;
}

void ClientSocket::dns_lookup(){
    struct addrinfo hints;
    memset(&hints, 0, sizeof(hints));
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
    }
}

size_t ClientSocket::accept_msg(TCPMessage* msg){
    size_t bytes_rx;
    bool r_n_found = false;
    char* buffer = msg->get_buffer();
    size_t rx_total = 0;

    while(!r_n_found){
        bytes_rx = recv(socket_fd, buffer + rx_total, 1, 0);
        if (bytes_rx <= 0){
            std::cerr << "ERR: NO DATA RECEIVED FROM SERVER." << std::endl;
            cleanup();
            exit(EXIT_FAILURE);
        }
        rx_total += 1;
        for(size_t i = 0; i < rx_total - 1; i++){
            if(buffer[i] == '\r' && buffer[i+1] == '\n'){
                r_n_found = true;
                break;
            }
        }
    }
    return rx_total;
}

bool validate_msg_open(client_info* info, TCPMessage outgoing_msg){

    if(outgoing_msg.get_msg_type() == JOIN){
        info->reply_msg_sent = true;
    } else if(outgoing_msg.get_msg_type() == AUTH){
        std::cerr << "ERR: Already authorized." << std::endl;
        return false;
    }
    return true;
}

void ClientSocket::start_tcp_chat(){
    create_socket();
    dns_lookup();
    establish_connection();

    struct pollfd fds[2];
    fds[0].fd = socket_fd;
    fds[0].events = POLLIN;
    fds[1].fd = STDIN_FILENO;
    fds[1].events = POLLIN;

    client_info info;

    if(get_socket_type() == SOCK_STREAM){

        while(true){
            int event_num = poll(fds, 2, -1);
            if (event_num == -1) {
                std::cerr << "ERR: POLL." << std::endl;
                cleanup();
                exit(EXIT_FAILURE);
            }
            for (int i = 0; i < 2; i++) {
                if (fds[i].revents & POLLIN) {
                    if (fds[i].fd == socket_fd) {
                        TCPMessage inbound_msg("", TO_BE_DECIDED);
                        size_t bytes_rx = accept_msg(&inbound_msg);
                        inbound_msg.process_inbound_msg(bytes_rx);
                        
                        if(info.client_state == START_STATE){
                            if(inbound_msg.get_msg_type() == ERR){
                                TCPMessage bye_msg("BYE", BYE);
                                bye_msg.proces_outgoing_msg();
                                send_msg(bye_msg);
                                cleanup();
                                exit(EXIT_SUCCESS);
                            }
                    
                        } else if(info.client_state == AUTH_STATE){
                            if(inbound_msg.get_msg_type() == REPLY_OK){
                                if(info.reply_msg_sent){
                                    inbound_msg.print_message();
                                    info.reply_msg_sent = false;
                                    info.client_state = OPEN_STATE;
                                }

                            } else if(inbound_msg.get_msg_type() == REPLY_NOK){
                                    if(info.reply_msg_sent){
                                        inbound_msg.print_message();
                                        info.reply_msg_sent = false;
                                    }
                            } else if(inbound_msg.get_msg_type() == ERR){
                                    TCPMessage bye_msg("BYE", BYE);
                                    bye_msg.proces_outgoing_msg();
                                    send_msg(bye_msg);
                                    cleanup();
                                    exit(EXIT_SUCCESS);
                            }
                        } else if(info.client_state == OPEN_STATE){
                            if(inbound_msg.get_msg_type() == ERR){
                                TCPMessage bye_msg("BYE", BYE);
                                bye_msg.proces_outgoing_msg();
                                send_msg(bye_msg);
                                cleanup();
                                exit(EXIT_SUCCESS);
                            } else if(inbound_msg.get_msg_type() == BYE){
                                cleanup();
                                exit(EXIT_SUCCESS);
                            } else if(inbound_msg.get_msg_type() == REPLY_NOK){
                                if(info.reply_msg_sent){
                                    inbound_msg.print_message();
                                    info.reply_msg_sent = false;
                                }
                                info.reply_msg_sent = false;
                            } else if(inbound_msg.get_msg_type() == REPLY_OK){
                                    if(info.reply_msg_sent){
                                        inbound_msg.print_message();
                                        info.reply_msg_sent = false;
                                    }

                            } else if(inbound_msg.get_msg_type() == MSG){
                                continue;
                            } else {
                                TCPMessage err_msg("Unknown or invalid message at current state.", ERR);
                                err_msg.set_display_name(info.dname);
                                err_msg.proces_outgoing_msg();
                                send_msg(err_msg);
                                TCPMessage bye_msg("BYE", BYE);
                                bye_msg.proces_outgoing_msg();
                                send_msg(bye_msg);
                                cleanup();
                                exit(EXIT_SUCCESS);
                            }
                        }
                    } else if (fds[i].fd == STDIN_FILENO) {
                        if(info.reply_msg_sent){
                            continue;
                        }
                        std::string message;
                        if(!std::getline(std::cin, message)){
                            TCPMessage bye_msg("BYE", BYE);
                            bye_msg.proces_outgoing_msg();
                            send_msg(bye_msg);
                            cleanup();
                            exit(EXIT_SUCCESS);
                        }

                        if(message.empty()){
                            continue;
                        }

                        TCPMessage outgoing_msg(message, USER_CMD);
                        outgoing_msg.set_display_name(info.dname);
                        outgoing_msg.proces_outgoing_msg();
                        
                        //Set username or change in case of rename command
                        if(outgoing_msg.get_msg_type() == AUTH || outgoing_msg.get_msg_type() == RENAME){
                            info.dname = outgoing_msg.get_display_name();
                        }

                        if(outgoing_msg.is_ready_to_send()){
                            if(info.client_state == START_STATE){
                                if(outgoing_msg.get_msg_type() != AUTH){
                                    std::cerr << "ERR: You must authorize first." << std::endl;
                                } else {
                                    info.reply_msg_sent = true;
                                    info.client_state = AUTH_STATE;
                                    send_msg(outgoing_msg);
                                }
                            } else if(info.client_state == AUTH_STATE){
                                if (outgoing_msg.get_msg_type() != AUTH && !info.reply_msg_sent){
                                    std::cerr << "ERR: Authorization wasnt succesful yet." << std::endl;
                                } else if (outgoing_msg.get_msg_type() == AUTH){
                                        info.reply_msg_sent = true;
                                        send_msg(outgoing_msg);
                                }
                            } else if(info.client_state == OPEN_STATE){
                                if(validate_msg_open(&info, outgoing_msg)){
                                    send_msg(outgoing_msg); 
                                }
                                    
                            }                               
                            
                        }   
                    }
                }    
            } 
        }
    }   
}

#include "socket.hpp"

ClientSocket::ClientSocket(int sock_type,  connection_info* parsed_info){

    socket_fd = -1;
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

int ClientSocket::get_epoll_fd(){
    return epoll_fd;
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
    int bytes_rx = recv(socket_fd, msg->get_buffer(), 1500, 0);
    if (bytes_rx <= 0){
      std::cerr << "ERR: NO DATA RECEIVED FROM SERVER." << std::endl;
      cleanup();
    }
    return bytes_rx;
}

bool validate_msg_open(client_info* info, TCPMessage outgoing_msg){
        if(info->awaiting_reply == true){
           info->msgQ.push(outgoing_msg);
        } else {
            if(outgoing_msg.get_msg_type() == JOIN){
                info->awaiting_reply = true;
            } else if(outgoing_msg.get_msg_type() == AUTH){
                std::cerr << "ERR: Already authorized." << std::endl;
            } else {
                return true;
            }
        
        }
        return false; 
}

void ClientSocket::start_tcp_chat(){
    create_socket();
    dns_lookup();
    establish_connection();

    struct epoll_event event;
    event.events = EPOLLIN;
    event.data.fd = get_socket_fd();
    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, get_socket_fd(), &event) != 0) {
        std::cerr << "ERR: EPOLL SOCKET CTL." << std::endl;
        cleanup();
        exit(EXIT_FAILURE);
    }

    event.data.fd = STDIN_FILENO;
    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, STDIN_FILENO, &event) != 0) {
        std::cerr << "ERR: EPOLL STDIN CTL." << std::endl;
        cleanup();
        exit(EXIT_FAILURE);
    }

    client_info info;

    if(get_socket_type() == SOCK_STREAM){

        while(true){
            struct epoll_event epl_events[2];
            int event_num = epoll_wait(epoll_fd, epl_events, 2, -1);
            if (event_num == -1) {
                std::cerr << "ERR: EPOLL WAIT." << std::endl;
                cleanup();
                exit(EXIT_FAILURE);
            }

            for(int event_idx = 0; event_idx < event_num; event_idx++){
                int current_fd = epl_events[event_idx].data.fd;

                if(current_fd == socket_fd){
                    TCPMessage inbound_msg("", TO_BE_DECIDED);
                    size_t bytes_rx = accept_msg(&inbound_msg);
                    inbound_msg.process_inbound_msg(bytes_rx);
                    
                    if(info.client_state == AUTH_STATE){
                        if(inbound_msg.get_msg_type() == REPLY_OK){
                            info.awaiting_reply = false;
                            info.auth_sent = false;
                            info.client_state = OPEN_STATE;
                            while(!info.msgQ.empty()){
                                send_msg(info.msgQ.front());
                                info.msgQ.pop();
                            } 
                        } else if(inbound_msg.get_msg_type() == REPLY_NOK){
                            info.awaiting_reply = true;
                            info.auth_sent = false;
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
                            info.awaiting_reply = true;
                            info.auth_sent = false;
                        } else if(inbound_msg.get_msg_type() == REPLY_OK){
                            info.awaiting_reply = false;
                            while(!info.msgQ.empty()){
                                TCPMessage outgoing_msg = info.msgQ.front();
                                if(validate_msg_open(&info, outgoing_msg)){
                                    send_msg(info.msgQ.front());
                                }
                                info.msgQ.pop();

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

                } else if(current_fd == STDIN_FILENO){
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
                                info.awaiting_reply = true;
                                info.auth_sent = true;
                                info.client_state = AUTH_STATE;
                                send_msg(outgoing_msg);
                            }
                        } else if(info.client_state == AUTH_STATE){
                            if (outgoing_msg.get_msg_type() != AUTH && !info.auth_sent && info.awaiting_reply){
                                std::cerr << "ERR: Authorization wasnt succesful yet." << std::endl;
                            } else if (info.auth_sent){
                                    info.msgQ.push(outgoing_msg);
                            } else if (outgoing_msg.get_msg_type() == AUTH){
                                    info.auth_sent = true;
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

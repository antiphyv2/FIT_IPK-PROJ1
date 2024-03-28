#include "clients.hpp"

NetworkClient::NetworkClient(connection_info* info) : conn_info(info), socket(new ClientSocket(info->sock_type)), dns_results(nullptr){}

NetworkClient::~NetworkClient(){
    delete socket;
    if(conn_info != nullptr){
        delete conn_info;
    }
    
    if(dns_results != nullptr){
        freeaddrinfo(dns_results);
    }
}

connection_info* NetworkClient::get_arg_info(){
    return conn_info;
}

struct addrinfo* NetworkClient::get_dns_info(){
    return dns_results;
}

ClientSocket* NetworkClient::get_socket(){
    return socket;
}

void NetworkClient::dns_lookup(){
    struct addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = socket->get_socket_type();
    hints.ai_protocol = 0;

    int retreived_info;
    if ((retreived_info = getaddrinfo(conn_info->ip_hostname.c_str(), conn_info->port.c_str(), &hints, &dns_results)) != 0){
        std::cout << "ERR: Could not resolve hostname." << std::endl;
        error_exit_program();
    }
}

void NetworkClient::establish_connection(){
    int ret_val;
    if((ret_val = connect(socket->get_socket_fd(), dns_results->ai_addr, dns_results->ai_addrlen)) != 0){
        std::cout << "ERR: Could not connect to the server." << std::endl;
        error_exit_program();
    }
}

void NetworkClient::send_msg(NetworkMessage& msg){
    ssize_t bytes_sent = send(socket->get_socket_fd(), msg.get_buffer(), msg.get_buffer_size(), 0);
    if (bytes_sent == -1) {
        std::cerr << "ERR: Message could not be send to server." << std::endl;
    }
}

void TCPClient::send_bye_and_exit(){
    TCPMessage bye_msg("BYE", BYE);
    bye_msg.process_outgoing_msg();
    send_msg(bye_msg);
    error_exit_program();
}

size_t NetworkClient::accept_msg(NetworkMessage* msg){
    size_t bytes_rx;
    bool r_n_found = false;
    char* buffer = msg->get_buffer();
    size_t rx_total = 0;

    while(!r_n_found || rx_total >= BUFFER_SIZE - 1){
        bytes_rx = recv(socket->get_socket_fd(), buffer + rx_total, 1, 0);
        if (bytes_rx <= 0){
            std::cerr << "ERR: NO DATA RECEIVED FROM SERVER." << std::endl;
            error_exit_program();
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

void TCPClient::start_tcp_chat(){
    socket->create_socket();
    dns_lookup();
    establish_connection();

    struct pollfd fds[2];
    fds[0].fd = socket->get_socket_fd();
    fds[0].events = POLLIN;
    fds[1].fd = STDIN_FILENO;
    fds[1].events = POLLIN;

    client_info info;

    while(true){
        int event_num = poll(fds, 2, -1);
        if (event_num == -1) {
            std::cerr << "ERR: POLL." << std::endl;
            error_exit_program();
        }
        for (int i = 0; i < 2; i++) {
            if (fds[i].revents & POLLIN) {
                if (fds[i].fd == socket->get_socket_fd()) {
                    TCPMessage inbound_msg("", TO_BE_DECIDED);
                    size_t bytes_rx = accept_msg(&inbound_msg);
                    inbound_msg.process_inbound_msg(bytes_rx);
                    
                    if(info.client_state == START_STATE){
                        if(inbound_msg.get_msg_type() == ERR){
                            send_bye_and_exit();
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
                            send_bye_and_exit();
                        }

                    } else if(info.client_state == OPEN_STATE){
                        if(inbound_msg.get_msg_type() == ERR){
                            send_bye_and_exit();
                        } else if(inbound_msg.get_msg_type() == BYE){
                            error_exit_program();
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
                            err_msg.process_outgoing_msg();
                            send_msg(err_msg);
                            send_bye_and_exit();
                        }
                    }
                } else if (fds[i].fd == STDIN_FILENO) {
                    if(info.reply_msg_sent){
                        continue;
                    }
                    std::string message;
                    if(!std::getline(std::cin, message)){
                        send_bye_and_exit();
                    }

                    if(message.empty()){
                        continue;
                    }

                    TCPMessage outgoing_msg(message, USER_CMD);
                    outgoing_msg.set_display_name(info.dname);
                    outgoing_msg.process_outgoing_msg();
                    
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
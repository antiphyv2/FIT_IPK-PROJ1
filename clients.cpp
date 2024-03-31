#include "clients.hpp"

NetworkClient::NetworkClient(connection_info* info) : conn_info(info), socket(new ClientSocket(info->sock_type)), dns_results(nullptr){}

NetworkClient::~NetworkClient(){}

UDPClient::UDPClient(connection_info* info) : NetworkClient(info){
    memset(&server_addr, 0, sizeof(server_addr));
    server_port = -1;
    confirm_msg_sent = false;
    change_server_port = true;
    memset(&server_addr, 0, sizeof(server_addr));
}

TCPClient::~TCPClient(){
    delete socket;
    if(conn_info != nullptr){
        delete conn_info;
    }
    
    if(dns_results != nullptr){
        freeaddrinfo(dns_results);
    }
}

UDPClient::~UDPClient(){
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

client_info* NetworkClient::get_cl_info(){
    return &cl_info;
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
        exit_program(false, EXIT_FAILURE);
    }
}

void TCPClient::establish_connection(){
    int ret_val;
    if((ret_val = connect(socket->get_socket_fd(), dns_results->ai_addr, dns_results->ai_addrlen)) != 0){
        std::cout << "ERR: Could not connect to the server." << std::endl;
        exit_program(false, EXIT_FAILURE);
    }
}

void TCPClient::send_msg(NetworkMessage& msg){
    int bytes_sent = send(socket->get_socket_fd(), msg.get_output_buffer(), msg.get_output_buffer_size(), 0);
    if (bytes_sent == -1) {
        std::cerr << "ERR: Message could not be send to server." << std::endl;
    }
}

void UDPClient::send_msg(NetworkMessage& msg){
    int bytes_sent = sendto(socket->get_socket_fd(), msg.get_output_buffer(), msg.get_output_buffer_size(), 0,  get_dns_info()->ai_addr, get_dns_info()->ai_addrlen);
    if(bytes_sent == -1){
        std::cerr << "ERR: Message could not be send to server." << std::endl;
    }
    std::cout << "BYTES SENT:" << bytes_sent << std::endl;
}

int TCPClient::accept_msg(NetworkMessage& msg){
    size_t bytes_rx;
    bool r_n_found = false;
    char* buffer = (char*) msg.get_input_buffer();
    size_t rx_total = 0;

    while(!r_n_found && rx_total <= BUFFER_SIZE - 1){
        bytes_rx = recv(socket->get_socket_fd(), buffer + rx_total, 1, 0);
        if (bytes_rx <= 0){
            std::cerr << "ERR: NO DATA RECEIVED FROM SERVER." << std::endl;
            exit_program(false, EXIT_FAILURE);
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

int UDPClient::accept_msg(NetworkMessage& msg){
    int bytes_rx;
    socklen_t addr_len = sizeof(server_addr);
    bytes_rx = recvfrom(socket->get_socket_fd(), msg.get_input_buffer(), BUFFER_SIZE, 0, (struct sockaddr*) &server_addr, &addr_len);
    if (bytes_rx <= 0){ 

        if(errno == EWOULDBLOCK || errno == EAGAIN){
            std::cerr << "ERR: TIMEOUT APPLIED." << std::endl;
        } else {
            std::cerr << "ERR: NO DATA RECEIVED FROM SERVER." << std::endl;
        }
        exit_program(false, EXIT_FAILURE);
    }
    server_port = ntohs(server_addr.sin_port);

    return bytes_rx;
}

bool validate_msg_open(client_info* info, NetworkMessage& outgoing_msg){

    if(outgoing_msg.get_msg_type() == JOIN){
        info->reply_msg_sent = true;
    } else if(outgoing_msg.get_msg_type() == AUTH){
        std::cerr << "ERR: Already authorized." << std::endl;
        return false;
    }
    return true;
}

void TCPClient::start_tcp_chat(){
    socket->create_socket(conn_info);
    std::cout << "SOCKET TIMEOUT:" << socket->get_socket_tv()->tv_usec << std::endl;
    dns_lookup();
    establish_connection();

    struct pollfd fds[2];
    fds[0].fd = socket->get_socket_fd();
    fds[0].events = POLLIN;
    fds[1].fd = STDIN_FILENO;
    fds[1].events = POLLIN;

    while(true){
        int nfds = 2;
        if(cl_info.reply_msg_sent){
            nfds = 1;
        }
        int ready_sockets = poll(fds, nfds, -1);
        if (ready_sockets == -1) {
            std::cerr << "ERR: POLL." << std::endl;
            exit_program(false, EXIT_FAILURE);
        }

        if (fds[0].revents & POLLIN) {
            TCPMessage inbound_msg("", TO_BE_DECIDED);
            int bytes_rx = accept_msg(inbound_msg);
            inbound_msg.process_inbound_msg(bytes_rx);


            if(cl_info.client_state == START_STATE){
                if(inbound_msg.get_msg_type() != TO_BE_DECIDED){
                    TCPMessage err_msg("Unknown or invalid message at current state.", ERR);
                    err_msg.set_display_name(cl_info.dname);
                    err_msg.process_outgoing_msg();
                    send_msg(err_msg);
                    //std::cerr << "ERR: Unknown message at current state." << std::endl;
                    exit_program(true, EXIT_FAILURE);
                }
        
            } else if(cl_info.client_state == AUTH_STATE){
                if(inbound_msg.get_msg_type() == REPLY_OK){
                    if(cl_info.reply_msg_sent){
                        inbound_msg.print_message();
                        cl_info.reply_msg_sent = false;
                        cl_info.client_state = OPEN_STATE;
                        continue;
                    }

                } else if(inbound_msg.get_msg_type() == REPLY_NOK){
                    if(cl_info.reply_msg_sent){
                        inbound_msg.print_message();
                        cl_info.reply_msg_sent = false;
                        continue;
                    }
                } else if(inbound_msg.get_msg_type() == ERR){
                    exit_program(true, EXIT_FAILURE);
                } else if(inbound_msg.get_msg_type() == BYE || inbound_msg.get_msg_type() == MSG || inbound_msg.get_msg_type() == INVALID_MSG){
                    std::cerr << "ERR: Unknown message at current state." << std::endl; 
                    TCPMessage err_msg("Unknown or invalid message at current state.", ERR);
                    err_msg.set_display_name(cl_info.dname);
                    err_msg.process_outgoing_msg();
                    send_msg(err_msg);
                    exit_program(true, EXIT_FAILURE);
                }

            } else if(cl_info.client_state == OPEN_STATE){
                if(inbound_msg.get_msg_type() == ERR){
                    exit_program(true, EXIT_FAILURE);
                } else if(inbound_msg.get_msg_type() == BYE){
                    exit_program(false, EXIT_SUCCESS);
                } else if(inbound_msg.get_msg_type() == REPLY_NOK){
                    if(cl_info.reply_msg_sent){
                        inbound_msg.print_message();
                        cl_info.reply_msg_sent = false;
                    }
                    cl_info.reply_msg_sent = false;
                } else if(inbound_msg.get_msg_type() == REPLY_OK){
                        if(cl_info.reply_msg_sent){
                            inbound_msg.print_message();
                            cl_info.reply_msg_sent = false;
                        }

                } else if(inbound_msg.get_msg_type() == MSG){
                    continue;
                } else {
                    TCPMessage err_msg("Unknown or invalid message at current state", ERR);
                    err_msg.set_display_name(cl_info.dname);
                    err_msg.process_outgoing_msg();
                    send_msg(err_msg);
                    exit_program(true, EXIT_FAILURE);
                }
            }
        }

        if(fds[1].revents & (POLLIN | POLLHUP)){
            std::string message;
            if(!std::getline(std::cin, message)){
                exit_program(true, EXIT_FAILURE);
            }

            if(message.empty()){
                continue;
            }

            TCPMessage outgoing_msg(message, USER_CMD);
            outgoing_msg.set_display_name(cl_info.dname);
            outgoing_msg.process_outgoing_msg();
            
            //Set username or change in case of rename command
            if(outgoing_msg.get_msg_type() == AUTH || outgoing_msg.get_msg_type() == RENAME){
                cl_info.dname = outgoing_msg.get_display_name();
            }

            if(outgoing_msg.is_ready_to_send()){
                if(cl_info.client_state == START_STATE){
                    if(outgoing_msg.get_msg_type() != AUTH){
                        std::cerr << "ERR: You must authorize first." << std::endl;
                    } else {
                        cl_info.reply_msg_sent = true;
                        cl_info.client_state = AUTH_STATE;
                        send_msg(outgoing_msg);
                    }
                } else if(cl_info.client_state == AUTH_STATE){
                    if (outgoing_msg.get_msg_type() != AUTH && !cl_info.reply_msg_sent){
                        std::cerr << "ERR: Authorization wasnt succesful yet." << std::endl;
                    } else if (outgoing_msg.get_msg_type() == AUTH){
                            cl_info.reply_msg_sent = true;
                            send_msg(outgoing_msg);
                    }
                } else if(cl_info.client_state == OPEN_STATE){
                    if(validate_msg_open(&cl_info, outgoing_msg)){
                        send_msg(outgoing_msg); 
                    }
                        
                }                               
                
            }   
        }    
    }
}


void UDPClient::start_udp_chat(){
    socket->create_socket(conn_info);
    std::cout << "SOCKET TIMEOUT:" << socket->get_socket_tv()->tv_usec << std::endl;
    dns_lookup();

    struct pollfd fds[2];
    fds[0].fd = socket->get_socket_fd();
    fds[0].events = POLLIN;
    fds[1].fd = STDIN_FILENO;
    fds[1].events = POLLIN;
    std::vector<uint16_t> confirm_id_vector;
    std::vector<uint16_t> reply_id_vector;
    std::vector<uint16_t> seen_ids;
    bool skip_message;

    while(true){
        int nfds = 2;
        if(cl_info.reply_msg_sent || confirm_msg_sent){
            nfds = 1;
        }
        int ready_sockets = poll(fds, nfds, -1);
        if (ready_sockets == -1) {
            std::cerr << "ERR: POLL." << std::endl;
            exit_program(false, EXIT_FAILURE);
        }
        
        if (fds[0].revents & POLLIN) {
            UDPMessage inbound_msg("", TO_BE_DECIDED, -1);
            int bytes_rx = accept_msg(inbound_msg);
            skip_message = inbound_msg.validate_unique_id(bytes_rx, seen_ids);
            if(skip_message){
                continue;
            }
            inbound_msg.process_inbound_msg(bytes_rx);
            std::cout << "BYTES_RX:" << bytes_rx << "MSG_TYPE" << inbound_msg.get_msg_type() << std::endl;

            if(cl_info.client_state == START_STATE){
                if(inbound_msg.get_msg_type() != TO_BE_DECIDED){
                    std::cerr << "ERR: Unknown message at current state." << std::endl;
                    exit_program(true, EXIT_FAILURE);
                }
        
            } else if(cl_info.client_state == AUTH_STATE){
                if(inbound_msg.get_msg_type() == CONFIRM){
                    uint16_t msg_id = inbound_msg.get_ref_msg_id();
                    std::cout << "MSG_ID_CONFIRM" << msg_id << std::endl;
                    if(confirm_id_vector.front() == msg_id){
                        confirm_id_vector.erase(confirm_id_vector.begin());
                        confirm_msg_sent = false;
                        std::cout << "CONFIRMED ID:" << msg_id << std::endl;
                        continue;
                    }

                } else if(inbound_msg.get_msg_type() == REPLY_OK){
                    seen_ids.push_back(inbound_msg.get_msg_id());
                    uint16_t msg_id = inbound_msg.get_ref_msg_id();

                    if(cl_info.reply_msg_sent){
                        if(change_server_port){
                            struct sockaddr_in* ip_address = (struct sockaddr_in*) dns_results->ai_addr;
                            ip_address->sin_port = htons(server_port);
                            change_server_port = false;
                        }
                        std::cout << "here" << reply_id_vector.front() << msg_id;
                        if(reply_id_vector.front() == msg_id){
                            reply_id_vector.erase(reply_id_vector.begin());
                            inbound_msg.print_message();
                            cl_info.reply_msg_sent = false;
                            cl_info.client_state = OPEN_STATE;
                            std::cout << "REPLY ID:" << msg_id << std::endl;
                        }
                    }

                    UDPMessage confirm_msg("", CONFIRM, inbound_msg.get_msg_id());
                    confirm_msg.process_outgoing_msg();
                    confirm_msg.get_msg_type();
                    send_msg(confirm_msg);
                    continue;

                } else if(inbound_msg.get_msg_type() == REPLY_NOK){
                    seen_ids.push_back(inbound_msg.get_msg_id());
                    uint16_t msg_id = inbound_msg.get_ref_msg_id();
                    if(cl_info.reply_msg_sent){
                        if(change_server_port){
                            struct sockaddr_in* ip_address = (struct sockaddr_in*) dns_results->ai_addr;
                            ip_address->sin_port = htons(server_port);
                            std::cout << "PORT:" << server_port << std::endl;
                            change_server_port = false;
                        }
                        
                        if(reply_id_vector.front() == msg_id){
                            reply_id_vector.erase(reply_id_vector.begin());
                            inbound_msg.print_message();
                            cl_info.reply_msg_sent = true;
                        }
                    }
                    UDPMessage confirm_msg("", CONFIRM, inbound_msg.get_msg_id());
                    confirm_msg.process_outgoing_msg();
                    confirm_msg.get_msg_type();
                    send_msg(confirm_msg);
                    //continue;

                } else if(inbound_msg.get_msg_type() == ERR){
                    UDPMessage confirm_msg("", CONFIRM, inbound_msg.get_msg_id());
                    confirm_msg.process_outgoing_msg();
                    confirm_msg.get_msg_type();
                    send_msg(confirm_msg);
                    exit_program(true, EXIT_FAILURE);
                } else if(inbound_msg.get_msg_type() == BYE || inbound_msg.get_msg_type() == MSG || inbound_msg.get_msg_type() == INVALID_MSG){
                    std::cerr << "ERR: Unknown message at current state." << std::endl; 
                    UDPMessage err_msg("Unknown or invalid message at current state.", ERR, cl_info.msg_counter);
                    err_msg.set_display_name(cl_info.dname);
                    err_msg.process_outgoing_msg();
                    send_msg(err_msg);
                    exit_program(true, EXIT_FAILURE);
                }

            } else if(cl_info.client_state == OPEN_STATE){
                if(inbound_msg.get_msg_type() == ERR){
                    UDPMessage confirm_msg("", CONFIRM, inbound_msg.get_msg_id());
                    confirm_msg.process_outgoing_msg();
                    confirm_msg.get_msg_type();
                    send_msg(confirm_msg);
                    exit_program(true, EXIT_FAILURE);
                    exit_program(true, EXIT_FAILURE);
                } else if(inbound_msg.get_msg_type() == BYE){
                    UDPMessage confirm_msg("", CONFIRM, inbound_msg.get_msg_id());
                    confirm_msg.process_outgoing_msg();
                    confirm_msg.get_msg_type();
                    send_msg(confirm_msg);
                    exit_program(true, EXIT_FAILURE);
                    exit_program(false, EXIT_SUCCESS);
                } else if(inbound_msg.get_msg_type() == CONFIRM){
                    if(confirm_id_vector.front() == inbound_msg.get_ref_msg_id()){
                        confirm_id_vector.erase(confirm_id_vector.begin());
                        confirm_msg_sent = false;
                        std::cout << "CONFIRMED ID:" << inbound_msg.get_ref_msg_id() << std::endl;
                        continue;
                    }
                } else if(inbound_msg.get_msg_type() == REPLY_NOK){
                    seen_ids.push_back(inbound_msg.get_msg_id());
                    if(cl_info.reply_msg_sent){
                        inbound_msg.print_message();
                        cl_info.reply_msg_sent = false;

                        if(reply_id_vector.front() == inbound_msg.get_ref_msg_id()){
                                reply_id_vector.erase(reply_id_vector.begin());
                                inbound_msg.print_message();
                                cl_info.reply_msg_sent = false;
                        }
                        UDPMessage confirm_msg("", CONFIRM, inbound_msg.get_msg_id());
                        confirm_msg.process_outgoing_msg();
                        confirm_msg.get_msg_type();
                        send_msg(confirm_msg);
                    }
                    std::cout <<"here OPEN NOK";
                    cl_info.reply_msg_sent = false;
                } else if(inbound_msg.get_msg_type() == REPLY_OK){
                    seen_ids.push_back(inbound_msg.get_msg_id());
                    if(cl_info.reply_msg_sent){
                        inbound_msg.print_message();
                        cl_info.reply_msg_sent = false;

                        if(reply_id_vector.front() == inbound_msg.get_ref_msg_id()){
                            reply_id_vector.erase(reply_id_vector.begin());
                            inbound_msg.print_message();
                            cl_info.reply_msg_sent = false;
                            std::cout << "REPLY ID:" << inbound_msg.get_ref_msg_id() << std::endl;
                        }
                        
                    UDPMessage confirm_msg("", CONFIRM, inbound_msg.get_msg_id());
                    confirm_msg.process_outgoing_msg();
                    confirm_msg.get_msg_type();
                    send_msg(confirm_msg);
                    continue;
                    }
                } else if(inbound_msg.get_msg_type() == MSG){
                    seen_ids.push_back(inbound_msg.get_msg_id());
                    UDPMessage confirm_msg("", CONFIRM, inbound_msg.get_msg_id());
                    confirm_msg.process_outgoing_msg();
                    confirm_msg.get_msg_type();
                    send_msg(confirm_msg);
                } else {
                    UDPMessage err_msg("Unknown or invalid message at current state.", ERR, cl_info.msg_counter);
                    err_msg.set_display_name(cl_info.dname);
                    err_msg.process_outgoing_msg();
                    send_msg(err_msg);
                    exit_program(true, EXIT_FAILURE);
                }
            }
        }

        if(fds[1].revents & (POLLIN | POLLHUP)){
            std::string message;
            if(!std::getline(std::cin, message)){
                exit_program(true, EXIT_FAILURE);
            }

            if(message.empty()){
                continue;
            }

            UDPMessage outgoing_msg(message, USER_CMD, cl_info.msg_counter);
            outgoing_msg.set_display_name(cl_info.dname);
            outgoing_msg.process_outgoing_msg();
            
            //Set username or change in case of rename command
            if(outgoing_msg.get_msg_type() == AUTH || outgoing_msg.get_msg_type() == RENAME){
                cl_info.dname = outgoing_msg.get_display_name();
            }

            if(outgoing_msg.is_ready_to_send()){
                cl_info.msg_counter++;
                if(cl_info.client_state == START_STATE){
                    if(outgoing_msg.get_msg_type() != AUTH){
                        std::cerr << "ERR: You must authorize first." << std::endl;
                    } else {
                        confirm_id_vector.push_back(outgoing_msg.get_msg_id());
                        reply_id_vector.push_back(outgoing_msg.get_msg_id());
                        cl_info.reply_msg_sent = true;
                        confirm_msg_sent = true;
                        cl_info.client_state = AUTH_STATE;
                        send_msg(outgoing_msg);
                    }
                } else if(cl_info.client_state == AUTH_STATE){
                    if (outgoing_msg.get_msg_type() != AUTH && !cl_info.reply_msg_sent){
                        std::cerr << "ERR: Authorization wasnt succesful yet." << std::endl;
                    } else if (outgoing_msg.get_msg_type() == AUTH){
                            confirm_id_vector.push_back(outgoing_msg.get_msg_id());
                            reply_id_vector.push_back(outgoing_msg.get_msg_id());
                            cl_info.reply_msg_sent = true;
                            confirm_msg_sent = true;
                            send_msg(outgoing_msg);
                    }
                } else if(cl_info.client_state == OPEN_STATE){
                    if(outgoing_msg.get_msg_type() == JOIN){
                        cl_info.reply_msg_sent = true;
                        confirm_msg_sent = true;
                        reply_id_vector.push_back(outgoing_msg.get_msg_id());
                    } else if(outgoing_msg.get_msg_type() == AUTH){
                        std::cerr << "ERR: Already authorized." << std::endl;
                        continue;
                    }
                    confirm_id_vector.push_back(outgoing_msg.get_msg_id());
                    send_msg(outgoing_msg);
                }                               
            }   
        }    
    }
}
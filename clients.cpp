/**
 * @file clients.cpp
 * @author xhejni00
 * @date 2024-04-01
 */

#include "clients.hpp"

NetworkClient::NetworkClient(connection_info* info) : conn_info(info), socket(new ClientSocket(info->sock_type)), dns_results(nullptr){}

NetworkClient::~NetworkClient(){}

UDPClient::UDPClient(connection_info* info) : NetworkClient(info){
    memset(&server_addr, 0, sizeof(server_addr));
    server_port = -1;
    confirm_msg_sent = false;
    change_server_port = true;
    retry_count = 0;
    timeout_happened = false;
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

std::vector<uint16_t>* UDPClient::get_seen_ids() {
        return &seen_ids;
}

int UDPClient::get_retry_count(){
    return retry_count;
}

void NetworkClient::dns_lookup(){
    struct addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = socket->get_socket_type();
    hints.ai_protocol = 0;

    //Obtain address from domain name
    int retreived_info;
    if ((retreived_info = getaddrinfo(conn_info->ip_hostname.c_str(), conn_info->port.c_str(), &hints, &dns_results)) != 0){
        std::cout << "ERR: Could not resolve hostname." << std::endl;
        exit_program(false, EXIT_FAILURE);
    }
}

void TCPClient::establish_connection(){
    int ret_val;
    //Connection to client in case of TCP
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
}

int TCPClient::accept_msg(NetworkMessage& msg){
    size_t bytes_rx;
    bool r_n_found = false;
    char* buffer = (char*) msg.get_input_buffer();
    size_t rx_total = 0;

    //Receiving by one byte until \r\n found
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
            return -1;
            
        } else {
            std::cerr << "ERR: NO DATA RECEIVED FROM SERVER." << std::endl;
            exit_program(false, EXIT_FAILURE);
        }
        
    }
    //save server port for later use (dynamic port change)
    server_port = ntohs(server_addr.sin_port);

    return bytes_rx;
}


void TCPClient::start_tcp_chat(){
    //Create socket object
    socket->create_socket(conn_info);
    //Translate domain name if needed
    dns_lookup();
    //Connect to the server
    establish_connection();

    //Preparing poll structure
    struct pollfd fds[2];
    fds[0].fd = socket->get_socket_fd();
    fds[0].events = POLLIN;
    fds[1].fd = STDIN_FILENO;
    fds[1].events = POLLIN;

    while(true){
        int nfds = 2;
        //Set poll fd number to 1 beacsue no reading from stdin unless reply received
        if(cl_info.reply_msg_sent){
            nfds = 1;
        }
        int ready_sockets = poll(fds, nfds, -1);
        if (ready_sockets == -1) {
            std::cerr << "ERR: POLL." << std::endl;
            exit_program(false, EXIT_FAILURE);
        }

        //Event from server
        if (fds[0].revents & POLLIN) {
            TCPMessage inbound_msg("", TO_BE_DECIDED);
            int bytes_rx = accept_msg(inbound_msg);
            inbound_msg.process_inbound_msg(bytes_rx);

            if(cl_info.client_state == START_STATE){
                if(inbound_msg.get_msg_type() != TO_BE_DECIDED){
                    TCPMessage err_msg("Unknown or invalid message at current state.", CUSTOM_ERR);
                    err_msg.set_display_name(cl_info.dname);
                    err_msg.process_outgoing_msg();
                    send_msg(err_msg);
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
                    //Send error message back and exit program
                    TCPMessage err_msg("Unknown or invalid message at current state.", CUSTOM_ERR);
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
                    //Send error msg back and exit
                    TCPMessage err_msg("Unknown or invalid message at current state", CUSTOM_ERR);
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
                //CTRL-D, exit program
                exit_program(true, EXIT_FAILURE);
            }
            
            //Skipping empty line
            if(message.empty()){
                continue;
            }

            //Outgoing message creation and handling user input msg
            TCPMessage outgoing_msg(message, USER_CMD);
            outgoing_msg.set_display_name(cl_info.dname);
            outgoing_msg.process_outgoing_msg();
            
            //Set username or change in case of rename command
            if(outgoing_msg.get_msg_type() == AUTH || outgoing_msg.get_msg_type() == RENAME){
                cl_info.dname = outgoing_msg.get_display_name();
            }

            //Message is supposed to be send
            if(outgoing_msg.is_ready_to_send()){
                if(cl_info.client_state == START_STATE){
                    if(outgoing_msg.get_msg_type() != AUTH){
                        std::cerr << "ERR: You must authorize first." << std::endl;
                    } else {
                        cl_info.reply_msg_sent = true;
                        //Auth state after sending message
                        cl_info.client_state = AUTH_STATE;
                        send_msg(outgoing_msg);
                    }
                } else if(cl_info.client_state == AUTH_STATE){
                    if (outgoing_msg.get_msg_type() != AUTH && !cl_info.reply_msg_sent){
                        //Reply wasnt ok, auth must be typed again
                        std::cerr << "ERR: Authorization wasnt succesful yet." << std::endl;
                    } else if (outgoing_msg.get_msg_type() == AUTH){
                            cl_info.reply_msg_sent = true;
                            send_msg(outgoing_msg);
                    }
                } else if(cl_info.client_state == OPEN_STATE){
                    if(outgoing_msg.get_msg_type() == JOIN){
                        cl_info.reply_msg_sent = true;
                        send_msg(outgoing_msg);
                    } else if(outgoing_msg.get_msg_type() != AUTH){
                        send_msg(outgoing_msg);
                    } else {
                        //Auth command is invalid after auth
                        std::cerr << "ERR: Already authorized." << std::endl;
                    }
                }                               
                
            }   
        }    
    }
}

void UDPClient::send_confim_exit(UDPMessage inbound_msg, bool exit){
    //Send confirm message
    UDPMessage confirm_msg("", CONFIRM, inbound_msg.get_msg_id());
    confirm_msg.process_outgoing_msg();
    send_msg(confirm_msg);
    //Program should also end
    if(exit){
        if(inbound_msg.get_msg_type() == BYE){
            exit_program(true, EXIT_SUCCESS);
        }
        exit_program(true, EXIT_FAILURE);
    }   
}

void UDPClient::send_error_exit(int* confirm_id, int* reply_id){
    //Make error message
    UDPMessage err_msg("Unknown or invalid message at current state.", CUSTOM_ERR, cl_info.msg_counter);
    err_msg.set_display_name(cl_info.dname);
    err_msg.process_outgoing_msg();
    //Increment counter and send it
    cl_info.msg_counter++;
    send_msg(err_msg);
    confirm_msg_sent = true;
    handle_timeout(confirm_id, reply_id, err_msg, true);  
}


void UDPClient::handle_timeout(int* confirm_id, int* reply_id, UDPMessage& outgoing_msg, bool exit){
    if(exit){
        exit_program(true, EXIT_FAILURE);
    }
    //No retries of sent message 
    int retry_number = 0;
    while(true){
        //create inbound message 
        UDPMessage inbound_msg("", TO_BE_DECIDED, -1);
        struct sockaddr_in server_addr;
        socklen_t addr_len = sizeof(server_addr);
        int bytes_rx = recvfrom(get_socket()->get_socket_fd(), inbound_msg.get_input_buffer(), BUFFER_SIZE, 0, (struct sockaddr*) &server_addr, &addr_len);
        if (bytes_rx <= 0){ 

            if(errno == EWOULDBLOCK || errno == EAGAIN){
                //retry limit reached
                if(retry_number == get_arg_info()->max_udp_retransmission){
                    std::cerr << "ERR: MAX TIMEOUTS REACHED." << std::endl;
                    exit_program(true, EXIT_FAILURE);
                }
                retry_number++;
                //send message again, wait for reply
                send_msg(outgoing_msg);
                continue;
            } else {
                std::cerr << "ERR: NO DATA RECEIVED FROM SERVER." << std::endl;
                break;
            }
        }

        //Validate message id and determine type
        bool skip_message = inbound_msg.validate_unique_id(bytes_rx, seen_ids);


        //if waiting for confirm, process it and exit if correct confirm was obtained
        if(confirm_msg_sent){
            if(inbound_msg.get_msg_type() == CONFIRM && inbound_msg.get_ref_msg_id() == outgoing_msg.get_msg_id()){
                confirm_msg_sent = false;
                break;
            }
        }
        //If message wasnt confirm, handle it over to the logic processor
        fsm_logic_handler(&skip_message, inbound_msg, confirm_id, reply_id, bytes_rx);
    }   
}

void UDPClient::fsm_logic_handler(bool* skip_message, UDPMessage& inbound_msg, int* confirm_id, int* reply_id, int bytes_rx){
    //Skips message if already seen
    if(*skip_message){
        return;
    }

    //Process the message
    inbound_msg.process_inbound_msg(bytes_rx);

    //Set ids to confirm or reply to from vector of ids
    if(!confirm_id_vector.empty()){
            *confirm_id = confirm_id_vector.front();
    } else {
        *confirm_id = -1;
    }
    if(!reply_id_vector.empty()){
        *reply_id = reply_id_vector.front();
    } else {
        *reply_id = -1;
    }

    if(cl_info.client_state == START_STATE){
        if(inbound_msg.get_msg_type() != TO_BE_DECIDED){
            std::cerr << "ERR: Unknown message at current state." << std::endl;
            exit_program(true, EXIT_FAILURE);
        }

    } else if(cl_info.client_state == AUTH_STATE){
        if(inbound_msg.get_msg_type() == CONFIRM){
            uint16_t msg_id = inbound_msg.get_ref_msg_id();
            //If ref id of confirm was expected, no longer waiting for confirm
            if(*confirm_id == msg_id){
                confirm_id_vector.erase(confirm_id_vector.begin());
                confirm_msg_sent = false;
            }
            return;

        } else if(inbound_msg.get_msg_type() == REPLY_OK){
            //Push id to vector since the message has already been seen
            seen_ids.push_back(inbound_msg.get_msg_id());
            uint16_t msg_id = inbound_msg.get_ref_msg_id();

            if(cl_info.reply_msg_sent){
                //If change of the port hasnt happened, switch the port so new messages will be sent to the new one
                if(change_server_port){
                    struct sockaddr_in* ip_address = (struct sockaddr_in*) dns_results->ai_addr;
                    ip_address->sin_port = htons(server_port);
                    change_server_port = false;
                }
                //If ref id of reply was expected, no longer waiting for reply, change state to open
                if(*reply_id == msg_id){
                    reply_id_vector.erase(reply_id_vector.begin());
                    inbound_msg.print_message();
                    cl_info.reply_msg_sent = false;
                    cl_info.client_state = OPEN_STATE;
                }
            }
            //send confirm 
            send_confim_exit(inbound_msg, false);
            return;

        } else if(inbound_msg.get_msg_type() == REPLY_NOK){
            seen_ids.push_back(inbound_msg.get_msg_id());
            uint16_t msg_id = inbound_msg.get_ref_msg_id();
            if(cl_info.reply_msg_sent){
                //If change of the port hasnt happened, switch the port so new messages will be sent to the new one
                if(change_server_port){
                    struct sockaddr_in* ip_address = (struct sockaddr_in*) dns_results->ai_addr;
                    ip_address->sin_port = htons(server_port);
                    change_server_port = false;
                }
                //If ref id of reply was expected, no longer waiting for reply, no change of state
                if(*reply_id == msg_id){
                    reply_id_vector.erase(reply_id_vector.begin());
                    inbound_msg.print_message();
                    cl_info.reply_msg_sent = false;
                }
            }
            send_confim_exit(inbound_msg, false);
            //continue;

        } else if(inbound_msg.get_msg_type() == ERR){
            send_confim_exit(inbound_msg, true);

        } else if(inbound_msg.get_msg_type() == MSG){
            UDPMessage confirm_msg("", CONFIRM, inbound_msg.get_msg_id());
            confirm_msg.process_outgoing_msg();
            send_msg(confirm_msg);
            send_error_exit(confirm_id, reply_id);
        }

    } else if(cl_info.client_state == OPEN_STATE){
        if(inbound_msg.get_msg_type() == ERR){
            inbound_msg.process_inbound_msg(bytes_rx);
            UDPMessage confirm_msg("", CONFIRM, inbound_msg.get_msg_id());
            confirm_msg.process_outgoing_msg();
            send_msg(confirm_msg);
            exit_program(true, EXIT_FAILURE);

        } else if(inbound_msg.get_msg_type() == BYE){
            inbound_msg.process_inbound_msg(bytes_rx);
            UDPMessage confirm_msg("", CONFIRM, inbound_msg.get_msg_id());
            confirm_msg.process_outgoing_msg();
            send_msg(confirm_msg);
            exit_program(false, EXIT_FAILURE);

        } else if(inbound_msg.get_msg_type() == CONFIRM){
            if(*confirm_id == inbound_msg.get_ref_msg_id()){
                confirm_id_vector.erase(confirm_id_vector.begin());
                confirm_msg_sent = false;
            }
            return;
        } else if(inbound_msg.get_msg_type() == REPLY_NOK){
            seen_ids.push_back(inbound_msg.get_msg_id());
            uint16_t msg_id = inbound_msg.get_ref_msg_id();
            if(cl_info.reply_msg_sent){
                if(*reply_id == msg_id){
                    reply_id_vector.erase(reply_id_vector.begin());
                    inbound_msg.print_message();
                    cl_info.reply_msg_sent = false;
                }
            }
            //Send confirm
            send_confim_exit(inbound_msg, false);
            
            
        } else if(inbound_msg.get_msg_type() == REPLY_OK){
            seen_ids.push_back(inbound_msg.get_msg_id());
            uint16_t msg_id = inbound_msg.get_ref_msg_id();

            if(cl_info.reply_msg_sent){
                if(*reply_id == msg_id){
                    reply_id_vector.erase(reply_id_vector.begin());
                    inbound_msg.print_message();
                    cl_info.reply_msg_sent = false;
                }
            }
            send_confim_exit(inbound_msg, false);
            return;

        } else if(inbound_msg.get_msg_type() == MSG){
            //Seen id and send confirm
            seen_ids.push_back(inbound_msg.get_msg_id());
            send_confim_exit(inbound_msg, false);
        } else {
            inbound_msg.process_inbound_msg(bytes_rx);
            UDPMessage confirm_msg("", CONFIRM, cl_info.msg_counter);
            confirm_msg.process_outgoing_msg();
            send_msg(confirm_msg);
            send_error_exit(confirm_id, reply_id);
        }
    }
}


void UDPClient::start_udp_chat(){
    socket->create_socket(conn_info);
    dns_lookup();
    
    //Poll structure
    struct pollfd fds[2];
    fds[0].fd = socket->get_socket_fd();
    fds[0].events = POLLIN;
    fds[1].fd = STDIN_FILENO;
    fds[1].events = POLLIN;
    
    bool skip_message; //Skip message if already seen
    int confirm_id; //Id of message to be confirmed
    int reply_id; //Id of message to be replied to
    std::vector<UDPMessage> messages_to_process; //Vector of messages to be processed

    while(true){
        int nfds = 2;
        //Set poll fd number to 1 beacsue no reading from stdin unless reply received
        if(cl_info.reply_msg_sent || confirm_msg_sent){
            nfds = 1;
        }
        int ready_sockets = poll(fds, nfds, -1);
        if (ready_sockets == -1) {
            std::cerr << "ERR: POLL." << std::endl;
            exit_program(false, EXIT_FAILURE);
        }

        //Socket event
        if ((fds[0].revents & POLLIN)) {
            //Accept message
            UDPMessage inbound_msg("", TO_BE_DECIDED, -1);
            int bytes_rx = accept_msg(inbound_msg);
            if(bytes_rx == -1){
                continue;
            }
            //Find message type and validate id
            skip_message = inbound_msg.validate_unique_id(bytes_rx, seen_ids);
            //std::cout << "MSG TYPE" << inbound_msg.get_msg_type() << std::endl;
            if(!skip_message){
                if(inbound_msg.get_msg_type() == INVALID_MSG || inbound_msg.get_msg_type() == ERR){
                    if(inbound_msg.get_msg_type() == ERR){
                        inbound_msg.process_inbound_msg(bytes_rx);
                        UDPMessage confirm_msg("", CONFIRM, inbound_msg.get_msg_id());
                        confirm_msg.process_outgoing_msg();
                        send_msg(confirm_msg);
                        exit_program(true, EXIT_FAILURE);
                    }
                    inbound_msg.process_inbound_msg(bytes_rx);
                    UDPMessage confirm_msg("", CONFIRM, cl_info.msg_counter);
                    confirm_msg.process_outgoing_msg();
                    send_msg(confirm_msg);
                    send_error_exit(&confirm_id, &reply_id);
                }

                if(cl_info.client_state == START_STATE && inbound_msg.get_msg_type() != TO_BE_DECIDED){
                    UDPMessage confirm_msg("", CONFIRM, inbound_msg.get_msg_id());
                    confirm_msg.process_outgoing_msg();
                    send_msg(confirm_msg);
                    send_error_exit(&confirm_id, &reply_id);     

                } else if(cl_info.client_state == AUTH_STATE){
                    if(inbound_msg.get_msg_type() == BYE || inbound_msg.get_msg_type() == MSG){
                        UDPMessage confirm_msg("", CONFIRM, inbound_msg.get_msg_id());
                        confirm_msg.process_outgoing_msg();
                        send_msg(confirm_msg);
                        send_error_exit(&confirm_id, &reply_id);
                    }
                }
            }
            //Handle message to the logic processor
            fsm_logic_handler(&skip_message, inbound_msg, &confirm_id, &reply_id, bytes_rx);
            continue;
        }

        if(fds[1].revents & (POLLIN | POLLHUP)){
            std::string message;
            if(!std::getline(std::cin, message)){
                //EOF
                exit_program(true, EXIT_FAILURE);
            }
            //Skipping empty line
            if(message.empty()){
                continue;
            }
            //Create object for outgoing message
            UDPMessage outgoing_msg(message, USER_CMD, cl_info.msg_counter);
            outgoing_msg.set_display_name(cl_info.dname);
            outgoing_msg.process_outgoing_msg();
            
            //Set username or change in case of rename command
            if(outgoing_msg.get_msg_type() == AUTH || outgoing_msg.get_msg_type() == RENAME){
                cl_info.dname = outgoing_msg.get_display_name();
            }

            //UDP message is ready to send
            if(outgoing_msg.is_ready_to_send()){
                cl_info.msg_counter++;
                if(cl_info.client_state == START_STATE){
                    if(outgoing_msg.get_msg_type() != AUTH){
                        std::cerr << "ERR: You must authorize first." << std::endl;
                    } else {
                        //Push message id to confirm and reply vector
                        confirm_id_vector.push_back(outgoing_msg.get_msg_id());
                        reply_id_vector.push_back(outgoing_msg.get_msg_id());
                        //Set confirm and reply send to true
                        cl_info.reply_msg_sent = true;
                        confirm_msg_sent = true;
                        //In auth state, client will wait for server confirm and reply
                        cl_info.client_state = AUTH_STATE;
                        send_msg(outgoing_msg); 
                        handle_timeout(&confirm_id, &reply_id, outgoing_msg, false);
                        
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
                            handle_timeout(&confirm_id, &reply_id, outgoing_msg, false);
                            
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
                    confirm_msg_sent = true;
                    handle_timeout(&confirm_id, &reply_id, outgoing_msg, false);
                    continue;
                }                               
            }   
        }    
    }
}
#include "main.hpp"
#include "messages.hpp"
#include "socket.hpp"
#include "arg_parser.hpp"
#include "clients.hpp"

NetworkClient* client_ptr;

void Signal_handler::graceful_exit(int signal){
    if(signal == SIGINT){
        exit_program(true, EXIT_SUCCESS);
    }
}

void exit_program(bool send_bye, int ret_state){
    if(send_bye){
        if(client_ptr->get_socket()->get_socket_type() == SOCK_STREAM){ //&& client_ptr->get_cl_info()->client_state != START_STATE){
            TCPMessage bye_msg("BYE", BYE);
            bye_msg.process_outgoing_msg();
            client_ptr->send_msg(bye_msg);
        } else {
            UDPMessage bye_msg("BYE", BYE, client_ptr->get_cl_info()->msg_counter);
            bye_msg.process_outgoing_msg();
            client_ptr->send_msg(bye_msg);
            while(true){
                UDPMessage inbound_msg("", TO_BE_DECIDED, -1);
                //int bytes_rx = client_ptr->accept_msg(inbound_msg);
                struct sockaddr_in server_addr;
                int bytes_rx;
                socklen_t addr_len = sizeof(server_addr);
                bytes_rx = recvfrom(client_ptr->get_socket()->get_socket_fd(), inbound_msg.get_input_buffer(), BUFFER_SIZE, 0, (struct sockaddr*) &server_addr, &addr_len);
                if (bytes_rx <= 0){ 

                    if(errno == EWOULDBLOCK || errno == EAGAIN){
                        std::cerr << "ERR: TIMEOUT APPLIED." << std::endl;
                        break;
                    }
                }
                std::vector<uint16_t> empty_vec;
                inbound_msg.validate_unique_id(bytes_rx, empty_vec);
                if(inbound_msg.get_msg_type() == CONFIRM && inbound_msg.get_ref_msg_id() == client_ptr->get_cl_info()->msg_counter){
                    std::cout << "CONFIRM ARRIVED:" << std::endl;
                    break;
                }
            }
        }
    }
    delete client_ptr;
    exit(ret_state);
}

int main(int argc, char* argv[]){
    std::signal(SIGINT, Signal_handler::graceful_exit);
    connection_info* info = CLI_Parser::parse_args(argc, argv);
    if(info->sock_type == SOCK_STREAM){
        TCPClient* client = new TCPClient(info); 
        client_ptr = client;
        client->start_tcp_chat();
    } else {
        UDPClient* client = new UDPClient(info);
        client_ptr = client;
        client->start_udp_chat();
    }
    //exit_program(false, EXIT_SUCCESS);
    return 0;
}
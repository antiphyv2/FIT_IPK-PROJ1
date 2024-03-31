#include "main.hpp"
#include "messages.hpp"
#include "socket.hpp"
#include "arg_parser.hpp"
#include "clients.hpp"

//Pointer to the client object to be able to exit and clean after CTRL-C signal
NetworkClient* client_ptr;

void Signal_handler::graceful_exit(int signal){
    if(signal == SIGINT){
        exit_program(true, EXIT_SUCCESS);
    }
}

void exit_program(bool send_bye, int ret_state){
    //Bye msg should be sent upon exiting
    if(send_bye){
        if(client_ptr->get_socket()->get_socket_type() == SOCK_STREAM){
            //TCP client will sent just simple bye message
            TCPMessage bye_msg("BYE", BYE);
            bye_msg.process_outgoing_msg();
            client_ptr->send_msg(bye_msg);
        } else {
            UDPMessage bye_msg("BYE", BYE, client_ptr->get_cl_info()->msg_counter);
            bye_msg.process_outgoing_msg();
            client_ptr->send_msg(bye_msg);
            //Wait for bye confirmation, then allow exit
            while(true){
                UDPMessage inbound_msg("", TO_BE_DECIDED, -1);
                struct sockaddr_in server_addr;
                socklen_t addr_len = sizeof(server_addr);
                int bytes_rx = recvfrom(client_ptr->get_socket()->get_socket_fd(), inbound_msg.get_input_buffer(), BUFFER_SIZE, 0, (struct sockaddr*) &server_addr, &addr_len);
                if (bytes_rx <= 0){ 

                    if(errno == EWOULDBLOCK || errno == EAGAIN){
                        std::cerr << "ERR: TIMEOUT APPLIED." << std::endl;
                        break;
                    } else {
                        std::cerr << "ERR: NO DATA RECEIVED FROM SERVER." << std::endl;
                        break;
                    }
                }
                std::vector<uint16_t> empty_vec;
                //Validate unique id will also check for msg type
                inbound_msg.validate_unique_id(bytes_rx, empty_vec);

                //Correct confirm arrived, client can now exit
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
    //CTRL-C SIGNAL
    std::signal(SIGINT, Signal_handler::graceful_exit);
    //Arg parsing
    connection_info* info = CLI_Parser::parse_args(argc, argv);
    //Creation of specific client based on provided user argument
    if(info->sock_type == SOCK_STREAM){
        TCPClient* client = new TCPClient(info); 
        client_ptr = client;
        client->start_tcp_chat();
    } else {
        UDPClient* client = new UDPClient(info);
        client_ptr = client;
        client->start_udp_chat();
    }
    return 0;
}
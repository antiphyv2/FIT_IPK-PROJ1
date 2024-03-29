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
    if(send_bye && client_ptr->get_socket()->get_socket_type() == SOCK_STREAM){ //&& client_ptr->get_cl_info()->client_state != START_STATE){
        TCPMessage bye_msg("BYE", BYE);
        bye_msg.process_outgoing_msg();
        client_ptr->send_msg(bye_msg);
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
        UDPMessage* msg_udp = new UDPMessage("/join discord", USER_CMD, 53213);
        msg_udp->set_display_name("MAREK");
        msg_udp->process_outgoing_msg();
        // msg_udp->clear_output_buffer();
        client->get_socket()->create_socket();
        client->dns_lookup();
        client->establish_connection();
        client->send_msg(*msg_udp);
        delete msg_udp;
        UDPMessage udp_recv("", TO_BE_DECIDED, 19122);
        size_t bytes_rx = client->accept_msg(udp_recv);
        udp_recv.process_inbound_msg(bytes_rx);
        std::cout << std::endl << "INCOMING MSG: ";
        udp_recv.print_message();
        std:: cout << "MSG TYPE:" << udp_recv.get_msg_type();
        std::cout << std::endl << "MSG ID: " << udp_recv.get_msg_id() << std::endl;;
    }
    exit_program(false, EXIT_SUCCESS);
    return 0;
}
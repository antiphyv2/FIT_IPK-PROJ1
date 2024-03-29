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
    if(send_bye && client_ptr->get_socket()->get_socket_type() == SOCK_STREAM){

        if(client_ptr->get_socket()->get_socket_type() == SOCK_STREAM){
            TCPMessage bye_msg("BYE", BYE);
            bye_msg.process_outgoing_msg();
            TCPClient* ptr = (TCPClient*) client_ptr;
            ptr->send_msg(bye_msg);
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
        // UDPClient* client = new UDPClient(info);
        // client_ptr = client;
        // UDPMessage msg_udp("/join discord", USER_CMD, 53213);
        // msg_udp.set_display_name("MAREK");
        // msg_udp.process_outgoing_msg();
        // UDPMessage msg_udp2("ahoj kamaradi", USER_CMD, 51213);
        // msg_udp2.set_display_name("DAVID");
        // msg_udp2.process_outgoing_msg();
        // UDPMessage msg_udp3("to je konec", ERR, 81);
        // msg_udp3.set_display_name("HONZA");
        // msg_udp3.process_outgoing_msg();
    }
    exit_program(false, EXIT_SUCCESS);
    return 0;
}
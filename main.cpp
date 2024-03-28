#include "main.hpp"
#include "messages.hpp"
#include "socket.hpp"
#include "arg_parser.hpp"
#include "clients.hpp"

NetworkClient* client_ptr;

void Signal_handler::graceful_exit(int signal){
    if(signal == SIGINT){
        exit_program(false, EXIT_SUCCESS);
    }
}

void exit_program(bool send_bye, int ret_state){
    if(send_bye){
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
    TCPClient* client = new TCPClient(info); 
    client_ptr = client;
    client->start_tcp_chat();
    Signal_handler::graceful_exit(SIGINT);
    return 0;
}
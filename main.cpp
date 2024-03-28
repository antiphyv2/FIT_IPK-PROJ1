#include "main.hpp"
#include "messages.hpp"
#include "socket.hpp"
#include "arg_parser.hpp"
#include "clients.hpp"

NetworkClient* client_ptr;

void Signal_handler::graceful_exit(int signal){
    if(signal == SIGINT){
        TCPMessage bye_msg("BYE", BYE);
        bye_msg.process_outgoing_msg();
        client_ptr->send_msg(bye_msg);
        delete client_ptr;
    }
    exit(EXIT_SUCCESS);
}

void error_exit_program(){
    delete client_ptr;
    exit(EXIT_FAILURE);
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
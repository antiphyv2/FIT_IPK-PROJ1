//#include "proj1.hpp"
#include "main.hpp"
#include "messages.hpp"
#include "socket.hpp"
#include "arg_parser.hpp"

ClientSocket* socket_ptr;

void Signal_handler::graceful_exit(int signal){
    if(signal == SIGINT){
        TCPMessage bye_msg("BYE", BYE);
        bye_msg.process_outgoing_msg();
        socket_ptr->send_msg(bye_msg);
        socket_ptr->cleanup();
    }
    exit(EXIT_SUCCESS);
}

int main(int argc, char* argv[]){
    std::signal(SIGINT, Signal_handler::graceful_exit);
    connection_info* info = CLI_Parser::parse_args(argc, argv);
    ClientSocket socket(info);
    socket_ptr = &socket;
    socket.start_tcp_chat();
    socket.cleanup();
    return 0;
}
//#include "proj1.hpp"
#include "proj1.hpp"
#include "messages.hpp"
#include "socket.hpp"

ClientSocket* socket_ptr;

int argument_parsing(int argc, char* argv[], connection_info* info){

int cli_arg, server_port;
int sock_type = -1;

    //Argument parsing
    while((cli_arg = getopt(argc, argv, "t:s:p:d:r:h")) != -1){
        switch (cli_arg)
        {
        case 't':
            info->protocol = optarg;

            if(info->protocol == "tcp"){
                sock_type = SOCK_STREAM;
            } else if (info->protocol == "udp"){
                sock_type = SOCK_DGRAM;
            } else{
                delete info;
                std::cerr << "ERR: You must select either tcp or udp protocol" << std::endl;
                exit(EXIT_FAILURE);
            }
            break;
        case 's':
            info->ip_hostname = optarg;
            break;
        case 'p':
            server_port = std::stoi(optarg, nullptr, 10);
            if(server_port < 0 || server_port > 65535){
                std::cerr << "ERR: Port out of range." << std::endl;
                exit(EXIT_FAILURE);
                delete info;
            }
            info->port = optarg;
            break;
        case 'd':
            info->udp_timeout = std::stoi(optarg, nullptr, 10);
            break;
        case 'r':
            info->max_udp_retransmission = std::stoi(optarg, nullptr, 10);
            break;
        case 'h':
            std::cout << "Usage: ./ipk -t <PROTOCOL> -s <SERVER IP> -p <PORT> -d <UDP_TIMEOUT> -r <UDP_RETRANSMISSION> " << std::endl;
            delete info;
            exit(EXIT_SUCCESS);
            break;
        default:
            break;
        }
    }
    return sock_type;
}

void Signal_handler::graceful_exit(int signal){
    if(signal == SIGINT){
        TCPMessage bye_msg("BYE", BYE);
        bye_msg.proces_outgoing_msg();
        socket_ptr->send_msg(bye_msg);
        socket_ptr->cleanup();
    }
    exit(EXIT_SUCCESS);
}

int main(int argc, char* argv[]){
    std::signal(SIGINT, Signal_handler::graceful_exit);
    connection_info* info = new connection_info();
    int sock_type = argument_parsing(argc, argv, info);
    ClientSocket socket(sock_type, info);
    socket_ptr = &socket;
    socket.start_tcp_chat();
    socket.cleanup();
    std::cout << "END OF PROGRAM.";
    return 0;
}
#include "proj1.hpp"

void print_server(server* server_info){
    std::cout << "Protocol:" << server_info->protocol << std::endl;
    std::cout << "IP/Hostname:" << server_info->ip_hostname << std::endl;
    std::cout << "Port:" << server_info->port << std::endl;
    std::cout << "UDP timeout:" << server_info->udp_timeout << std::endl;
    std::cout << "Max UDP retransmission:" << server_info->max_udp_retransmission << std::endl;
}


void argument_parsing(int argc, char* argv[], server* server_info){
    int cli_arg;

    //Argument parsing
    while((cli_arg = getopt(argc, argv, "t:s:p:d:r:h")) != -1){
        switch (cli_arg)
        {
        case 't':
            server_info->protocol = optarg;
            break;
        case 's':
            server_info->ip_hostname = optarg;
            break;
        case 'p':
            server_info->port = std::stoi(optarg, nullptr, 10);
            break;
        case 'd':
            server_info->udp_timeout = std::stoi(optarg, nullptr, 10);
            break;
        case 'r':
            server_info->max_udp_retransmission = std::stoi(optarg, nullptr, 10);
            break;
        case 'h':
            std::cout << "Usage: ./ipk -t <PROTOCOL> -s <SERVER IP> -p <PORT> -d <UDP_TIMEOUT> -r <UDP_RETRANSMISSION> " << std::endl;
            exit(EXIT_SUCCESS);
            break;
        default:
            break;
        }
    }
}

int main(int argc, char* argv[]){
    server* server_info = new server;
    argument_parsing(argc, argv, server_info);
    print_server(server_info);
    std::cout << "END OF PROGRAM.";
    return 0;
}
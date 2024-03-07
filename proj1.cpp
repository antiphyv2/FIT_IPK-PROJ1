#include "proj1.hpp"


void print_server(server* server_info){
    std::cout << "Protocol:" << server_info->protocol << std::endl;
    std::cout << "IP/Hostname:" << server_info->ip_hostname << std::endl;
    std::cout << "Port:" << server_info->port << std::endl;
    std::cout << "UDP timeout:" << server_info->udp_timeout << std::endl;
    std::cout << "Max UDP retransmission:" << server_info->max_udp_retransmission << std::endl;
}


void argument_parsing(int argc, char* argv[], server* server_info){
    int cli_arg, server_port;

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
            server_port = std::stoi(optarg, nullptr, 10);
            if(server_port < 0 || server_port > 65535){
                std::cerr << "ERR: Port out of range.";
                exit(EXIT_FAILURE);
            }
            server_info->port = optarg;
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
    server* server_info = new server{};
    argument_parsing(argc, argv, server_info);
    print_server(server_info);

    struct addrinfo hints = {0};
    struct addrinfo* results;
    //std::memset(&hints, 0, sizeof(hints));
    //std::cout << hints.ai_addr << std::endl;
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_protocol = 0;

    int retreived_info;
    if ((retreived_info = getaddrinfo(server_info->ip_hostname.c_str(), server_info->port.c_str(), &hints, &results)) != 0){
        std::cout << "ERR: Could not resolve hostname." << std::endl;
        //exit(EXIT_FAILURE);
    }

    std::cout << server_info->ip_hostname.c_str() << std::endl;

    std::cout << "CREATING SOCKET..." << std::endl;

    int socket_fd, socket_close;

    if((socket_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1){
        std::cerr << "ERROR CREATING SOCKET." << std::endl;
        exit(EXIT_FAILURE);
    }

    std::cout << "SOCKET SUCCESFULLY CREATED." << std::endl;

    if((socket_close = close(socket_fd)) == -1){
        std::cerr << "ERROR CLOSING SOCKET." << std::endl;
        exit(EXIT_FAILURE);
    }

    std::cout << "SOCKET SUCCESFULLY CLOSED." << std::endl;

    delete server_info;

    std::cout << "END OF PROGRAM.";
    return 0;
}
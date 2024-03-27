#include "arg_parser.hpp"

connection_info* CLI_Parser::parse_args(int argc, char* argv[]){
    connection_info* cli_info = new connection_info();
    int cli_arg, server_port;

        //Argument parsing
    while((cli_arg = getopt(argc, argv, "t:s:p:d:r:h")) != -1){
    switch (cli_arg){
        case 't':
            cli_info->protocol = optarg;

            if(cli_info->protocol == "tcp"){
                cli_info->sock_type = SOCK_STREAM;
            } else if (cli_info->protocol == "udp"){
                cli_info->sock_type = SOCK_DGRAM;
            } else{
                delete cli_info;
                std::cerr << "ERR: You must select either tcp or udp protocol" << std::endl;
                exit(EXIT_FAILURE);
            }
            break;
        case 's':
            cli_info->ip_hostname = optarg;
            break;
        case 'p':
            server_port = std::stoi(optarg, nullptr, 10);
            if(server_port < 0 || server_port > 65535){
                std::cerr << "ERR: Port out of range." << std::endl;
                exit(EXIT_FAILURE);
                delete cli_info;
            }
            cli_info->port = optarg;
            break;
        case 'd':
            cli_info->udp_timeout = std::stoi(optarg, nullptr, 10);
            break;
        case 'r':
            cli_info->max_udp_retransmission = std::stoi(optarg, nullptr, 10);
            break;
        case 'h':
            std::cout << "Usage: ./ipk -t <PROTOCOL> -s <SERVER IP> -p <PORT> -d <UDP_TIMEOUT> -r <UDP_RETRANSMISSION> " << std::endl;
            delete cli_info;
            exit(EXIT_SUCCESS);
            break;
        default:
            break;
        }
    }

    return cli_info;
}
/**
 * @file arg_parser.cpp
 * @author xhejni00
 * @date 2024-04-01
 */
#include "arg_parser.hpp"

connection_info* CLI_Parser::parse_args(int argc, char* argv[]){
    connection_info* cli_info = new connection_info();
    int cli_arg;
    int server_port = 0;

    //getopt arg parsing
    while((cli_arg = getopt(argc, argv, "t:s:p:d:r:h")) != -1){
    switch (cli_arg){
        case 't':
            if(strcmp(optarg, "tcp") == 0){
                cli_info->sock_type = SOCK_STREAM;
            } else if (strcmp(optarg, "udp") == 0){
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
                delete cli_info;
                exit(EXIT_FAILURE);
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
        default:
            break;
        }
    }

    if(cli_info->sock_type != SOCK_DGRAM && cli_info->sock_type != SOCK_STREAM){
        std::cerr << "ERR: No protocol selected." << std::endl;
        delete cli_info;
        exit(EXIT_FAILURE);
    }
    
    //If no values were provided, default values are set
    if(cli_info->udp_timeout == 0){
        cli_info->udp_timeout = 250;
    }

    if(cli_info->max_udp_retransmission == 0){
        cli_info->max_udp_retransmission = 3;
    }
    if(server_port == 0){
        cli_info->port = "4567";
    }
    return cli_info;
}
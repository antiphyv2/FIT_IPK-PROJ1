/**
 * @file arg_parser.hpp
 * @author xhejni00
 * @date 2024-04-01
 */
#ifndef ARG_PARSER_HPP
#define ARG_PARSER_HPP

#include "main.hpp"

//Structure for cli arguments
typedef struct INFO{
    std::string ip_hostname;
    std::string port;
    uint16_t udp_timeout;
    uint16_t max_udp_retransmission;
    uint16_t sock_type;
} connection_info;

class CLI_Parser{

public:
    /**
     * @brief 
     * 
     * @param argc number of cli args
     * @param argv array of cli args
     * @return connection_info* pointer to the structure with args
     */
    static connection_info* parse_args(int argc, char* argv[]);
};
#endif
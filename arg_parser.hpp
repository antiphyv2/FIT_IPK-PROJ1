#ifndef ARG_PARSER_HPP
#define ARG_PARSER_HPP

#include "main.hpp"

typedef struct INFO{
    std::string ip_hostname;
    std::string port;
    uint16_t udp_timeout;
    uint16_t max_udp_retransmission;
    uint16_t sock_type;
} connection_info;

class CLI_Parser{

public:
    static connection_info* parse_args(int argc, char* argv[]);
};
#endif
#include <iostream>
#include <unistd.h>
#include <string>

typedef struct INFO{
    std::string protocol;
    std::string ip_hostname;
    uint16_t port;
    uint16_t udp_timeout;
    uint16_t max_udp_retransmission;
} server;

void argument_parsing(int argc, char* argv[], server* server_info);
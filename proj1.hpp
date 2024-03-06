#include <iostream>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <limits.h>
#include <stdbool.h>
#include <string>
#include <cstring>

typedef struct INFO{
    std::string protocol;
    std::string ip_hostname;
    std::string port;
    uint16_t udp_timeout;
    uint16_t max_udp_retransmission;
} server;

typedef enum {
    AUTH,
    JOIN,
    ERR,
    BYE,
    MSG,
    REPLY,
    NOT_REPLY,
} states;

void argument_parsing(int argc, char* argv[], server* server_info);
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
#include <sstream>
#include <vector>
#include <csignal>

typedef struct INFO{
    std::string protocol;
    std::string ip_hostname;
    std::string port;
    uint16_t udp_timeout;
    uint16_t max_udp_retransmission;
} connection_info;

typedef enum {
    START_STATE,
    AUTH_STATE,
    OPEN_STATE,
    ERROR_STATE,
    END_STATE,
} states;

int argument_parsing(int argc, char* argv[], connection_info* info);

void graceful_exit(int signal);
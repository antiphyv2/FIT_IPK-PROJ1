#ifndef MAIN_HPP
#define MAIN_HPP

#include <iostream>
#include <sys/epoll.h>
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
#include <queue>
#include <poll.h>
#include <cctype>

typedef struct INFO{
    std::string ip_hostname;
    std::string port;
    uint16_t udp_timeout;
    uint16_t max_udp_retransmission;
    uint16_t sock_type;
} connection_info;

typedef enum {
    START_STATE,
    AUTH_STATE,
    OPEN_STATE,
    ERROR_STATE,
    END_STATE,
} fsm_states;

class Signal_handler{
    public:
        static void graceful_exit(int signal);
};

int argument_parsing(int argc, char* argv[], connection_info* info);
#endif
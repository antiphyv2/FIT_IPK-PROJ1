#ifndef MAIN_HPP
#define MAIN_HPP

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
#include <queue>
#include <poll.h>
#include <cctype>

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

#endif
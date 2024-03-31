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

//FSM states needed for parsing logic
typedef enum {
    START_STATE,
    AUTH_STATE,
    OPEN_STATE,
} fsm_states;

class Signal_handler{
    public:
        /**
         * @brief Handles CTRL-C signal
         * 
         * @param signal CTRL-C == SIGINT
         */
        static void graceful_exit(int signal);
};

/**
 * @brief Function that handles program exit
 * 
 * @param send_bye flag if bye will be send to server
 * @param ret_state return SUCCESS or FAILURE
 */
void exit_program(bool send_bye, int ret_state);
#endif
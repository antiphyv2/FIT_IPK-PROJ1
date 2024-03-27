#ifndef ARG_PARSER_HPP
#define ARG_PARSER_HPP

#include "main.hpp"

class CLI_Parser{

public:
    static connection_info* parse_args(int argc, char* argv[]);
};
#endif
#include "proj1.hpp"

class ClientSocket{
    public:

    private:
        int socket_fd; //Socket FD
        int type; //TCP or UDP
        struct addrinfo* dns_results; //Structure for DNS retrieval
        connection_info* info; //CLI arguments

    public:
        ClientSocket(int sock_type);
        void cleanup();

        int get_socket_fd();
        int get_socket_type();
        void set_arg_info(connection_info* info);
        struct addrinfo* get_dns_info();
        connection_info* get_arg_info();

        void dns_lookup();
        void print_args();
};
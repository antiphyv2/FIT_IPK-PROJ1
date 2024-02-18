#include <iostream>
#include <unistd.h>
#include <string>

using namespace std;


typedef struct INFO{
    string protocol;
    string ip_hostname;
    uint16_t port;
    uint16_t udp_timeout;
    uint16_t max_udp_retransmission;
} server;


void print_server(server* server_info){
    cout << "Protocol:" << server_info->protocol << endl;
    cout << "IP/Hostname:" << server_info->ip_hostname << endl;
    cout << "Port:" << server_info->port << endl;
    cout << "UDP timeout:" << server_info->udp_timeout << endl;
    cout << "Max UDP retransmission:" << server_info->max_udp_retransmission << endl;

}


int main(int argc, char* argv[]){
    server server_info;
    int cli_arg;
    while((cli_arg = getopt(argc, argv, "t:s:p:d:r:h")) != -1){
        switch (cli_arg)
        {
        case 't':
            server_info.protocol = optarg;
            break;
        case 's':
            server_info.ip_hostname = optarg;
            break;
        case 'p':
            server_info.port = stoi(optarg, nullptr, 10);
            break;
        case 'd':
            server_info.udp_timeout = stoi(optarg, nullptr, 10);
            break;
        case 'r':
            server_info.max_udp_retransmission = stoi(optarg, nullptr, 10);
            break;
        case 'h':
            std::cout << "Zde bude napoveda." << std::endl;
            break;
        default:
            break;
        }
    }
    print_server(&server_info);
    std::cout << "END OF PROGRAM.";
    return 0;
}
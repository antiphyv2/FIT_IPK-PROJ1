//#include "proj1.hpp"
#include "messages.hpp"

int argument_parsing(int argc, char* argv[], connection_info* info){

int cli_arg, server_port;
int sock_type = -1;

    //Argument parsing
    while((cli_arg = getopt(argc, argv, "t:s:p:d:r:h")) != -1){
        switch (cli_arg)
        {
        case 't':
            info->protocol = optarg;

            if(info->protocol == "tcp"){
                sock_type = SOCK_STREAM;
            } else if (info->protocol == "udp"){
                sock_type = SOCK_DGRAM;
            } else{
                delete info;
                exit(EXIT_FAILURE);
            }
            break;
        case 's':
            info->ip_hostname = optarg;
            break;
        case 'p':
            server_port = std::stoi(optarg, nullptr, 10);
            if(server_port < 0 || server_port > 65535){
                std::cerr << "ERR: Port out of range.";
                exit(EXIT_FAILURE);
                delete info;
            }
            info->port = optarg;
            break;
        case 'd':
            info->udp_timeout = std::stoi(optarg, nullptr, 10);
            break;
        case 'r':
            info->max_udp_retransmission = std::stoi(optarg, nullptr, 10);
            break;
        case 'h':
            std::cout << "Usage: ./ipk -t <PROTOCOL> -s <SERVER IP> -p <PORT> -d <UDP_TIMEOUT> -r <UDP_RETRANSMISSION> " << std::endl;
            delete info;
            exit(EXIT_SUCCESS);
            break;
        default:
            break;
        }
    }

    return sock_type;
}

int main(int argc, char* argv[]){
    connection_info* info = new connection_info();
    int sock_type = argument_parsing(argc, argv, info);
    ClientSocket socket(sock_type);
    socket.set_arg_info(info);
    socket.print_args();
    socket.dns_lookup();

    // if (connect(tcp_socket.get_socket_fd(), tcp_socket.get_dns_info()->ai_addr, tcp_socket.get_dns_info()->ai_addrlen) != 0){
    //     std::cerr << "ERROR ESTABLISHING CONNECTION." << std::endl;
    //     tcp_socket.cleanup();
    //     exit(EXIT_FAILURE);
    // }
    if(socket.get_socket_type() == SOCK_STREAM){
        int i = 0;
        std::string dname = "";
        while(i < 1){
            std::string message;
            std::getline(std::cin, message);
            TCPMessage output_message(message, dname);
            output_message.process_local_msg();
            output_message.add_line_ending();
            
            if(output_message.get_msg_type() == AUTH || output_message.get_msg_type() == RENAME){
                dname = output_message.get_display_name();
            }
            if(output_message.is_ready_to_send()){
                output_message.print_buffer();
                std::cout << output_message.get_buffer_size() << std::endl;
            }
            i++;
        }


        socket.cleanup();
        std::cout << "END OF PROGRAM.";
        return 0;
    }

}
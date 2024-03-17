#include "messages.hpp"

TCPMessage::TCPMessage(std::string input_msg){
    type = NOTHING;
    message_to_process = input_msg;
    buffer[0] = '\0';
}

void TCPMessage::print_message(){
    std::cout << message_to_process << std::endl;
}

void TCPMessage::fill_output_buffer(std::string msg_part){
    size_t length = msg_part.length();
    size_t position = std::strlen(buffer);
    std::strncpy(buffer + position, msg_part.c_str(), length);
    buffer[length + position] = '\0'; 
}

void TCPMessage::add_line_ending(){
    size_t length = std::strlen(buffer);
    buffer[length] = '\r';
    buffer[length + 1] = '\n';
    buffer[length + 2] = '\0';
}

const char* TCPMessage::get_buffer(){
    return buffer;
}

size_t TCPMessage::get_buffer_size(){
    return std::strlen(buffer);
}

void TCPMessage::print_buffer(){

    std::cout << buffer << std::endl;
}
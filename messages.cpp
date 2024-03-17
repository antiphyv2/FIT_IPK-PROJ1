#include "messages.hpp"

TCPMessage::TCPMessage(std::string input_msg, std::string dname){
    type = NOTHING;
    local_msg = input_msg;
    msg_from_server = "";
    display_name = dname;

    buffer[0] = '\0';
}

void TCPMessage::process_local_msg(){
    std::istringstream TCP_message(local_msg);
    std::string fragment;
    std::string support_string;
    int word_order = 1;
    ready_to_send = true;

    std::vector<std::string> msg_fragments;
    while(TCP_message >> fragment){
        msg_fragments.push_back(fragment);
    }

    for(const auto& fragment : msg_fragments){
        if(type == NOTHING){
            if(fragment == "/auth"){
                type = AUTH;
                add_to_buffer("AUTH ");
            } else if(fragment == "/join"){
                type = JOIN;
                add_to_buffer("JOIN ");
            } else if(fragment == "/rename"){
                type = RENAME;
            } else if (fragment == "/help"){
                type = HELP;
                add_to_buffer("Available commands:\n");
                add_to_buffer("/auth {Username} {Secret} {DisplayName}\n");
                add_to_buffer("/join {ChannelID}\n");
                add_to_buffer("/rename {DisplayName}\n");
                add_to_buffer("/help for showing help.");
                ready_to_send = false;
                break;
            } else {    
                type = MSG;
                add_to_buffer("MSG FROM ");
            }
        } else if(type == AUTH){
            if(msg_fragments.size() != 4){
                ready_to_send = false;
                std::cerr << "Wrong command syntax. Usage: /auth {Username} {Secret} {DisplayName}" << std::endl;
                break;
            }

            if(word_order == 1){
                if(validate_msg_param(fragment, "ID")){
                    add_to_buffer(fragment);
                    add_to_buffer(" AS ");
                    display_name = fragment;
                    word_order++;
                } else {
                    ready_to_send = false;
                    std::cerr << "Wrong command syntax. Usage: /auth {Username} {Secret} {DisplayName}" << std::endl;;
                    break;
                }
            } else if(word_order == 2){
                if(validate_msg_param(fragment, "SECRET")){
                    support_string = fragment;
                    word_order++;
                } else {
                    ready_to_send = false;
                    std::cerr << "Wrong command syntax. Usage: /auth {Username} {Secret} {DisplayName}" << std::endl;;
                    break;
                }
            } else{
                if(validate_msg_param(fragment, "DNAME")){
                    add_to_buffer(fragment);
                    add_to_buffer(" USING ");
                    add_to_buffer(support_string);
                } else {
                    ready_to_send = false;
                    std::cerr << "Wrong command syntax. Usage: /auth {Username} {Secret} {DisplayName}" << std::endl;;
                    break;
                }
            }
        } else if(type == JOIN){
            if(msg_fragments.size() != 2){
                ready_to_send = false;
                std::cerr << "Wrong command syntax. Usage: /join {ChannelID}" << std::endl;
                break;
            }
            if(validate_msg_param(fragment, "ID")){
                add_to_buffer(fragment);
                add_to_buffer(" AS ");
                add_to_buffer(display_name);
            } else {
                ready_to_send = false;
                std::cerr << "Wrong command syntax. Usage: /join {ChannelID}" << std::endl;
                break;
            }
        } else if(type == RENAME){
            ready_to_send = false;
            if(validate_msg_param(fragment, "ID")){
                display_name = fragment;
                
            } else {
                std::cerr << "Wrong command syntax. Usage: /rename {DisplayName}" << std::endl;
                break;
            }
        }
    }
}

bool TCPMessage::validate_msg_param(std::string parameter, std::string pattern){
    return true;
}

bool TCPMessage::is_ready_to_send(){
    return ready_to_send;
}

void TCPMessage::print_local_message(){
    std::cout << local_msg << std::endl;
}

void TCPMessage::print_msg_from_server(){
    std::cout << msg_from_server << std::endl;
}

void TCPMessage::add_to_buffer(std::string msg_part){
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

std::string TCPMessage::get_display_name(){
    return display_name;
}

void TCPMessage::set_display_name(std::string name){
    display_name = name;
}

msg_types TCPMessage::get_msg_type(){
    return type;
}

const char* TCPMessage::get_buffer(){
    return buffer;
}

size_t TCPMessage::get_buffer_size(){
    return std::strlen(buffer);
}

void TCPMessage::print_buffer(){

    std::cout << buffer;
}


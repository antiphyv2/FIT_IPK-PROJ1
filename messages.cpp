#include "messages.hpp"

TCPMessage::TCPMessage(std::string input_msg, msg_types msg_type){
    type = msg_type;
    message = input_msg;
    display_name = "";
    buffer[0] = '\0';
}

void TCPMessage::proces_outgoing_msg(){
    std::istringstream TCP_message(message);
    std::string fragment;
    std::string support_string;
    int msg_part_counter = 1;
    ready_to_send = true;

    std::vector<std::string> msg_fragments;
    while(TCP_message >> fragment){
        msg_fragments.push_back(fragment);
    }

    for(const auto& fragment : msg_fragments){
        if(type == USER_CMD){
            if(fragment == "/auth"){
                if(msg_fragments.size() != 4){
                    ready_to_send = false;
                    std::cerr << "ERR: Wrong command syntax. Usage: /auth {Username} {Secret} {DisplayName}" << std::endl;
                    break;
                }
                type = AUTH;
                add_to_buffer("AUTH ");
            } else if(fragment == "/join"){
                if(msg_fragments.size() != 2){
                    ready_to_send = false;
                    std::cerr << "ERR: Wrong command syntax. Usage: /join {ChannelID}" << std::endl;
                    break;
                }
                type = JOIN;
                add_to_buffer("JOIN ");
            } else if(fragment == "/rename"){
                if(msg_fragments.size() != 2){
                    ready_to_send = false;
                    std::cerr << "ERR: Wrong command syntax. Usage: /rename {DisplayName}" << std::endl;
                    break;
                }
                type = RENAME;
            } else if (fragment == "/help"){
                type = HELP;
                ready_to_send = false;
                std::cout << "Available commands:\n/auth {Username} {Secret} {DisplayName}\n/join {ChannelID}\n/rename {DisplayName}\n/help for showing help." << std::endl;
                break;
            } else {    
                type = MSG;
                    add_to_buffer("MSG FROM ");
                    add_to_buffer(display_name);
                    add_to_buffer(" IS ");
                    if(validate_msg_param(message, "MSG")){
                        add_to_buffer(message);
                    } else {
                        std::cerr << "ERR: Wrong message format or length." << std::endl;
                    }
                    break;
            }
        } else if(type == AUTH){
            if(msg_part_counter == 1){
                if(validate_msg_param(fragment, "ID")){
                    add_to_buffer(fragment);
                    add_to_buffer(" AS ");
                    msg_part_counter++;
                } else {
                    ready_to_send = false;
                    std::cerr << "ERR: Wrong command syntax. Usage: /auth {Username} {Secret} {DisplayName}" << std::endl;;
                    break;
                }
            } else if(msg_part_counter == 2){
                if(validate_msg_param(fragment, "SECRET")){
                    support_string = fragment;
                    msg_part_counter++;
                } else {
                    ready_to_send = false;
                    std::cerr << "ERR: Wrong command syntax. Usage: /auth {Username} {Secret} {DisplayName}" << std::endl;;
                    break;
                }
            } else{
                if(validate_msg_param(fragment, "DNAME")){
                    add_to_buffer(fragment);
                    add_to_buffer(" USING ");
                    add_to_buffer(support_string);
                    display_name = fragment;
                } else {
                    ready_to_send = false;
                    std::cerr << "ERR: Wrong command syntax. Usage: /auth {Username} {Secret} {DisplayName}" << std::endl;;
                    break;
                }
            }
        } else if(type == JOIN){
            if(validate_msg_param(fragment, "ID")){
                add_to_buffer(fragment);
                add_to_buffer(" AS ");
                add_to_buffer(display_name);
            } else {
                ready_to_send = false;
                std::cerr << "ERR: Wrong command syntax. Usage: /join {ChannelID}" << std::endl;
                break;
            }
        } else if(type == RENAME){
            ready_to_send = false;
            if(validate_msg_param(fragment, "ID")){
                display_name = fragment;
                
            } else {
                std::cerr << "ERR: Wrong command syntax. Usage: /rename {DisplayName}" << std::endl;
                break;
            }
        } else if(type == ERR){
            add_to_buffer("ERR FROM ");
            add_to_buffer(display_name);
            add_to_buffer(" IS ");
            if(validate_msg_param(message, "MSG")){
                add_to_buffer(message);
            } else {
                ready_to_send = false;
                std::cerr << "ERR: Wrong message format or length." << std::endl;
            }
            break;
        } else if(type == BYE){
            add_to_buffer("BYE");
            break;
        }
    }
    add_line_ending();
}

void TCPMessage::process_inbound_msg(size_t bytes_rx){
    std::string help_string(buffer, bytes_rx);
    std::istringstream server_msg(std::string(buffer, bytes_rx));
    std::string msg_part;
    std::vector<std::string> msg_vector;
    while(server_msg >> msg_part){
        msg_vector.push_back(msg_part);
    }

    if(msg_vector.size() == 0){
        type = ERR;
        std::cerr << "ERR: Empty server msg." << std::endl;
        return;
    }

    if(type == TO_BE_DECIDED){
        for (char &character : msg_vector[0]) {
            character = std::toupper(character);
        }

        if(msg_vector.size() == 1 && msg_vector[0] == "BYE"){
            type = BYE;
            return;
        }

        if(msg_vector.size() >= 4){
            std::string message_to_extract;

            if(msg_vector[0] == "REPLY"){
                for (char &character : msg_vector[1]) {
                    character = std::toupper(character);
                }

                if(msg_vector[1] == "OK"){
                    type = REPLY_OK;

                    std::regex pattern("is", std::regex_constants::icase);
                    std::smatch match_regex;
                    if (std::regex_search(help_string, match_regex, pattern)) {
                        message_to_extract = help_string.substr(match_regex.position() + 3); //length of is + 1 for whitespace
                    } else {
                        type = ERR;
                        std::cerr << "ERR: Unknown incoming message from server" << std::endl;
                        return;
                    }
                    std::string reply_msg = "Success: ";
                    reply_msg.append(message_to_extract);
                    message = reply_msg;
                    return;
                } else if(msg_vector[1] == "NOK"){
                    type = REPLY_NOK;

                    std::regex pattern("is", std::regex_constants::icase);
                    std::smatch match_regex;
                    if (std::regex_search(help_string, match_regex, pattern)) {
                        message_to_extract = help_string.substr(match_regex.position() + 3); //length of is + 1 for whitespace
                    } else {
                        type = ERR;
                        std::cerr << "ERR: Unknown incoming message from server" << std::endl;
                        return;
                    }
                    std::string reply_msg = "Failure: ";
                    reply_msg.append(message_to_extract);
                    message = reply_msg;
                    return;
                }
            } else if(msg_vector[0] == "ERR"){
                type = ERR;
                std::cerr << "ERR FROM " << msg_vector[2] << ": " << msg_vector[4] << std::endl;
                return;
            } else if(msg_vector[0] == "MSG"){
                type = MSG;

                std::regex pattern("is", std::regex_constants::icase);
                std::smatch match_regex;
                if (std::regex_search(help_string, match_regex, pattern)) {
                    message_to_extract = help_string.substr(match_regex.position() + 3); //length of is + 1 for whitespace
                } else {
                    std::cerr << "ERR: Unknown incoming message from server" << std::endl;
                    return;
                }

                std::cout << msg_vector[2] << ": " << message_to_extract; //<< std::endl;
                return;
            }
        } 

        type = ERR;
        std::cerr << "ERR: Unknown incoming message from server" << std::endl;
        return;
    }
}

bool TCPMessage::validate_msg_param(std::string parameter, std::string pattern){
    if(pattern == "ID" || pattern == "SECRET"){
        if(pattern == "ID"){
            if(parameter.size() > 20){
                return false;
            }
        } else {
            if(parameter.size() > 128){
                return false;
            }
        }

        for(auto ch : parameter){
            if(!std::isalnum(ch) && ch != '-'){
                return false;
            }
        }
        return true;

    } else if(pattern == "DNAME"){
        if(parameter.size() > 20){
            return false;
        }

        for(auto ch : parameter){
            //Check if in printable characters
            if(!(ch >= '!' && ch <= '~')){
                return false;
            }
        }
        return true;

    } else if(pattern == "MSG"){
        if(parameter.size() > 1400){
            return false;
        }

        for(auto ch : parameter){
            //Check if in printable characters
            if(!((ch >= '!' && ch <= '~') || ch == ' ')){
                return false;
            }
        }
        return true;

    } else {
        return false;
    }
}

bool TCPMessage::is_ready_to_send(){
    return ready_to_send;
}

void TCPMessage::print_message(){
    std::cout << message;
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

void TCPMessage::set_msg_type(msg_types msg_type){
    type = msg_type;
}

char* TCPMessage::get_buffer(){
    return buffer;
}

size_t TCPMessage::get_buffer_size(){
    return std::strlen(buffer);
}

void TCPMessage::print_buffer(){
    std::cout << buffer;
}

void TCPMessage::clear_buffer(){
    memset(buffer, 0, sizeof(buffer));
}


#include "messages.hpp"

NetworkMessage::NetworkMessage(std::string input_msg, msg_types msg_type) : type(msg_type), ready_to_send(false), display_name(""), message(input_msg){
    buffer[0] = '\0';
}     

bool NetworkMessage::is_ready_to_send(){
    return ready_to_send;
}

void NetworkMessage::print_message(){
    if(type == REPLY_OK || type == REPLY_NOK){
        std::cerr << message << std::endl;
    } else {
        std::cout << message << std::endl;
    }
}

std::string NetworkMessage::get_display_name(){
    return display_name;
}

void NetworkMessage::set_display_name(std::string name){
    display_name = name;
}

msg_types NetworkMessage::get_msg_type(){
    return type;
}

void NetworkMessage::set_msg_type(msg_types msg_type){
    type = msg_type;
}

char* NetworkMessage::get_input_buffer(){
    return buffer;
}

size_t NetworkMessage::get_input_buffer_size(){
    return std::strlen(buffer);
}

void* TCPMessage::get_output_buffer(){
    return buffer;
}

size_t TCPMessage::get_output_buffer_size(){
    return std::strlen(buffer);
}

void* UDPMessage::get_output_buffer(){
    return udp_buffer.data();
}

size_t UDPMessage::get_output_buffer_size(){
    return udp_buffer.size();
}

void UDPMessage::clear_output_buffer(){
    udp_buffer.clear();
}

void NetworkMessage::print_input_buffer(){
    std::cout << buffer;
}

void NetworkMessage::clear_buffer(){
    memset(buffer, 0, sizeof(buffer));
}

uint16_t UDPMessage::get_msg_id(){
    return message_id;
}

void NetworkMessage::check_user_message(std::vector<std::string>& message_parts){
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
            } else if(fragment == "/join"){
                if(msg_fragments.size() != 2){
                    ready_to_send = false;
                    std::cerr << "ERR: Wrong command syntax. Usage: /join {ChannelID}" << std::endl;
                    break;
                }
                type = JOIN;
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
                    if(!validate_msg_param(message, "MSG")){
                        std::cerr << "ERR: Wrong message format or length." << std::endl;
                    }
                    break;
            }
        } else if(type == AUTH){
            if(msg_part_counter == 1){
                if(validate_msg_param(fragment, "ID")){
                    message_parts.push_back(fragment);
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
                    message_parts.push_back(fragment);
                    message_parts.push_back(support_string);
                    display_name = fragment;
                } else {
                    ready_to_send = false;
                    std::cerr << "ERR: Wrong command syntax. Usage: /auth {Username} {Secret} {DisplayName}" << std::endl;;
                    break;
                }
            }
        } else if(type == JOIN){
            if(validate_msg_param(fragment, "ID")){
                message_parts.push_back(fragment);
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
            if(!validate_msg_param(message, "MSG")){
                ready_to_send = false;
                std::cerr << "ERR: Wrong message format or length." << std::endl;
            }
            break;
        } else if(type == BYE){
            break;
        }
    }
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

void TCPMessage::remove_line_ending(std::string& message){
    if(!message.empty() && message.back() == '\n'){
        message.pop_back();
    }
    if(!message.empty() && message.back() == '\r'){
        message.pop_back();
    }
}

void TCPMessage::process_outgoing_msg(){
    std::vector<std::string> msg_parts;
    check_user_message(msg_parts);

   if(type == AUTH){
    add_to_buffer("AUTH ");
    add_to_buffer(msg_parts.front());
    msg_parts.erase(msg_parts.begin());
    add_to_buffer(" AS ");
    add_to_buffer(msg_parts.front());
    msg_parts.erase(msg_parts.begin());
    add_to_buffer(" USING ");
    add_to_buffer(msg_parts.back());
   } else if(type == JOIN){
    add_to_buffer("JOIN ");
    add_to_buffer(msg_parts.back());
    add_to_buffer(" AS ");
    add_to_buffer(display_name);
   } else if(type == ERR){
    add_to_buffer("ERR FROM ");
    add_to_buffer(display_name);
    add_to_buffer(" IS ");
    add_to_buffer(message);
   } else if(type == MSG){
    add_to_buffer("MSG FROM ");
    add_to_buffer(display_name);
    add_to_buffer(" IS ");
    add_to_buffer(message);
   } else if(type == BYE){
    add_to_buffer("BYE");
   } else {
    return;
   }
    add_line_ending();
}

void UDPMessage::process_outgoing_msg(){

    std::vector<std::string> msg_parts;
    check_user_message(msg_parts);

    if(type == AUTH){
        udp_buffer.push_back(UDP_AUTH);
        udp_buffer.push_back(message_id >> 8);
        udp_buffer.push_back(message_id & 0xFF);
        for (char c : msg_parts.front()){
            udp_buffer.push_back(static_cast<uint8_t>(c));
        }
        msg_parts.erase(msg_parts.begin());
        udp_buffer.push_back('\0');
        for (char c : msg_parts.front()){
            udp_buffer.push_back(static_cast<uint8_t>(c));
        }
        msg_parts.erase(msg_parts.begin());
        udp_buffer.push_back('\0');
        for (char c : msg_parts.front()){
            udp_buffer.push_back(static_cast<uint8_t>(c));
        }
        msg_parts.erase(msg_parts.begin());
        udp_buffer.push_back('\0');
    } else if(type == JOIN){
        udp_buffer.push_back(UDP_JOIN);
        udp_buffer.push_back(message_id >> 8);
        udp_buffer.push_back(message_id & 0xFF);
        for (char c : msg_parts.back()){
            udp_buffer.push_back(static_cast<uint8_t>(c));
        }
        udp_buffer.push_back('\0');
        for (char c : display_name){
            udp_buffer.push_back(static_cast<uint8_t>(c));
        }
        udp_buffer.push_back('\0');
    } else if(type == MSG || type == ERR){
        if(type == ERR){
            udp_buffer.push_back(UDP_ERR);
        } else {
            udp_buffer.push_back(UDP_MSG);
        }
        udp_buffer.push_back(message_id >> 8);
        udp_buffer.push_back(message_id & 0xFF);
        for (char c : display_name){
            udp_buffer.push_back(static_cast<uint8_t>(c));
        }
        udp_buffer.push_back('\0');
        for (char c : message){
            udp_buffer.push_back(static_cast<uint8_t>(c));
        }
        udp_buffer.push_back('\0');

    } else if(type == BYE){
        udp_buffer.push_back(UDP_BYE);
        udp_buffer.push_back(message_id >> 8);
        udp_buffer.push_back(message_id & 0xFF);
        for (char c : display_name){
            udp_buffer.push_back(static_cast<uint8_t>(c));
        }
        //udp_buffer.push_back('\0');
    } else {
        return;
    }

    for (auto byte : udp_buffer) {
    std::cout << std::hex << std::setw(2) << std::setfill('0') << static_cast<unsigned>(byte) << " ";
    }
    std::cout << std::dec << std::endl; 
}

void UDPMessage::process_inbound_msg(int bytes_rx){
    if(bytes_rx < 3){
        type = ERR;
        std::cerr << "ERR: Unknown incoming message from server" << std::endl;
        return;
    }
    uint8_t type_to_compare = buffer[0];
    memcpy(&message_id, buffer + 1, sizeof(message_id));

    if(type_to_compare == UDP_CONFIRM){
        type = CONFIRM;
    } else if(type_to_compare == UDP_REPLY){
        if(bytes_rx < 8){
            type = ERR;
            std::cerr << "ERR: Unknown incoming message from server" << std::endl;
            return;
        }
        if(buffer[3] == '\0'){
            type = REPLY_NOK;
            message.append("Failure: ");
        } else {
            type = REPLY_OK;
            message.append("Success: ");
        }
        memcpy(&ref_message_id, buffer + 4, sizeof(ref_message_id));

        int start_pos = 6;
        for (; start_pos < bytes_rx; start_pos++) {
            if (buffer[start_pos] == '\0') {
                break; 
            }
            message += buffer[start_pos];
        }

    } else if(type_to_compare == UDP_MSG || type_to_compare == UDP_ERR){
        if(type_to_compare == UDP_MSG){
            type = MSG;
        } else {
            type = ERR;
        }
        
        if(bytes_rx < 7){
            type = ERR;
            std::cerr << "ERR: Unknown incoming message from server" << std::endl;
            return;
        }
        std::string acquired_dname;
        int start_pos = 3;
        for (; start_pos < bytes_rx; start_pos++) {
            if (buffer[start_pos] == '\0') {
                start_pos += 1;
                break; 
            }
            acquired_dname += buffer[start_pos];
        }
        std::string acquired_msg;
        for (; start_pos < bytes_rx; start_pos++) {
            if (buffer[start_pos] == '\0') {
                break; 
            }
            acquired_msg += buffer[start_pos];
        }
        if(type == MSG){
            std::cout << acquired_dname << ": " << acquired_msg << std::endl;
        } else {
            std::cerr << "ERR FROM " << acquired_dname << ": " << acquired_msg << std::endl;
        }
    } else if(type_to_compare == UDP_BYE){
        type = BYE;
    } else {
        type = ERR;
        std::cerr << "ERR: Unknown incoming message from server" << std::endl;
        return;
    }
}

void TCPMessage::process_inbound_msg(int bytes_rx){
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
                    remove_line_ending(message_to_extract);
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
                    remove_line_ending(message_to_extract);
                    reply_msg.append(message_to_extract);
                    message = reply_msg;
                    return;
                }
            } else if(msg_vector[0] == "ERR" || msg_vector[0] == "MSG"){
                if(msg_vector[0] == "ERR"){
                    type = ERR;
                } else {
                    type = MSG;
                }
                std::regex pattern("is", std::regex_constants::icase);
                std::smatch match_regex;
                if (std::regex_search(help_string, match_regex, pattern)) {
                    message_to_extract = help_string.substr(match_regex.position() + 3); //length of is + 1 for whitespace
                } else {
                    std::cerr << "ERR: Unknown incoming message from server" << std::endl;
                    return;
                }
                remove_line_ending(message_to_extract);
                if(type == ERR){
                    std::cerr << "ERR FROM " << msg_vector[2] << ": " << message_to_extract << std::endl;
                } else {
                    std::cout << msg_vector[2] << ": " << message_to_extract << std::endl;
                }
                return;
            }
        } 

        type = ERR;
        std::cerr << "ERR: Unknown incoming message from server" << std::endl;
        return;
    }
}

bool NetworkMessage::validate_msg_param(std::string parameter, std::string pattern){
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


//
// Created by Graham Hill on 2025/01/06.
//

#ifndef COMMANDS_H
#define COMMANDS_H

#include <unordered_map>

// Enum for commands
enum Command {
    CMD_ECHO,
    CMD_TYPE,
    CMD_EXIT_0,
    CMD_UNKNOWN,
    CMD_PWD
};

// Function to map string commands to enum
inline Command get_command(const std::string& word) {
    static const std::unordered_map<std::string, Command> command_map = {
        {"echo", CMD_ECHO},
        {"type", CMD_TYPE},
        {"exit", CMD_EXIT_0}, // We handle "exit 0" separately
        {"pwd", CMD_PWD}
    };

    const auto it = command_map.find(word);
    return it != command_map.end() ? it->second : CMD_UNKNOWN;
}


#endif //COMMANDS_H

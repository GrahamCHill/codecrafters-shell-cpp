#include <iostream>
#include <sstream>
#include <unordered_map>
#include <string>
#include <cstdlib>
#include <vector>
#include <unistd.h> // For access function

#include "terminal_highlight.h" // Ensure this header is correctly included

// Enum for commands
enum Command {
    CMD_ECHO,
    CMD_TYPE,
    CMD_EXIT_0,
    CMD_UNKNOWN
};

// Function to map string commands to enum
Command get_command(const std::string& word) {
    static const std::unordered_map<std::string, Command> command_map = {
        {"echo", CMD_ECHO},
        {"type", CMD_TYPE},
        {"exit", CMD_EXIT_0} // We handle "exit 0" separately
    };

    const auto it = command_map.find(word);
    return it != command_map.end() ? it->second : CMD_UNKNOWN;
}

// Function to split a string by a delimiter
std::vector<std::string> split(const std::string& str, const char delimiter) {
    std::vector<std::string> tokens;
    std::string token;
    std::istringstream tokenStream(str);
    while (std::getline(tokenStream, token, delimiter)) {
        tokens.push_back(token);
    }
    return tokens;
}

// Function to check if a file exists and is executable
bool is_executable(const std::string& path) {
    return access(path.c_str(), X_OK) == 0;
}

int main() {
    // Flush after every std::cout / std::cerr
    std::cout << std::unitbuf;
    std::cerr << std::unitbuf;

    while (true) {
        // Display the prompt
        std::cout << "$ ";

        // Read input
        std::string input;
        std::getline(std::cin, input);

        // Parse the first word (command) and the rest (arguments)
        std::istringstream iss(input);
        std::string first_word;
        iss >> first_word;

        switch (get_command(first_word)) {
            case CMD_ECHO: {
                // Extract the remaining input after the command
                std::string remaining;
                std::getline(iss, remaining);
                if (!remaining.empty()) {
                    // Remove leading spaces before output
                    remaining.erase(0, remaining.find_first_not_of(' '));
                    std::cout << remaining << std::endl; // Print the remaining text
                } else {
                    std::cout << RED << "echo: missing argument" << RESET << std::endl;
                }
                break;
            }
            case CMD_TYPE: {
                std::string second_word;
                iss >> second_word;

                if (!second_word.empty()) {
                    // Check if the second word is a shell builtin
                    if (get_command(second_word) != CMD_UNKNOWN) {
                        std::cout << second_word << " is a shell builtin" << std::endl;
                    } else {
                        // Search for the command in PATH
                        if (const char* path_env = std::getenv("PATH")) {
                            std::string path_var(path_env);
                            std::vector<std::string> directories = split(path_var, ':');
                            bool found = false;

                            for (const auto& dir : directories) {
                                std::string full_path = dir + "/";
                                full_path.append(second_word);
                                if (is_executable(full_path)) {
                                    std::cout << second_word << " is " << full_path << std::endl;
                                    found = true;
                                    break;
                                }
                            }

                            if (!found) {
                                std::cout << RED << second_word << RESET << ": not found" << std::endl;
                            }
                        } else {
                            std::cout << RED << "PATH not set" << RESET << std::endl;
                        }
                    }
                } else {
                    std::cout << RED << second_word << RESET << ": not found" << std::endl;
                }
                break;
            }
            case CMD_EXIT_0: {
                std::string argument;
                iss >> argument;

                if (argument == "0") {
                    return 0; // Exit successfully
                } else {
                    std::cout << RED << input << RESET << ": not found" << std::endl;
                }
                break;
            }
            case CMD_UNKNOWN:
            default:
                std::cout << RED << input << RESET << ": not found" << std::endl;
                break;
        }
    }

}

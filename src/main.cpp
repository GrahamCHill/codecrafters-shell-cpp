#include <iostream>
#include <sstream>
#include <unordered_map>
#include <string>

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
                if (iss.peek() != EOF) {
                    std::string remaining;
                    std::getline(iss, remaining);
                    remaining.erase(0, remaining.find_first_not_of(' ')); // Remove leading spaces
                    std::cout << remaining << std::endl; // Print the remaining text
                } else {
                    std::cout << RED << input << RESET << ": command not found" << std::endl;
                }
                break;
            }
            case CMD_TYPE: {
                std::string second_word;
                iss >> second_word;

                if (!second_word.empty() && get_command(second_word) != CMD_UNKNOWN) {
                    std::cout << second_word << " is a shell builtin" << std::endl;
                } else {
                    std::cout << RED << input << RESET << ": command not found" << std::endl;
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

    return 0;
}

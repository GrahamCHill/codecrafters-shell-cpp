#include <iostream>
#include <sstream>
#include <string>


#include "terminal_highlight.h" // Ensure this header is correctly included
#include "commands.h"
#include "helper_functions.h"

int main() {
    set_initial_directory_command();
    std::string currentDirectory = get_current_directory_command();
    set_current_directory_command(currentDirectory);
    // Flush after every std::cout / std::cerr
    std::cout << std::unitbuf;
    std::cerr << std::unitbuf;

    while (true) {
        // Display the prompt
        std::cout << GREEN << "$ " << RESET;

        // Read input
        std::string input;
        std::getline(std::cin, input);

        // Parse the first word (command) and the rest (arguments)
        std::istringstream iss(input);
        std::string first_word;
        iss >> first_word;

        // Check if it's a built-in command
        switch (get_command(first_word)) {
            case CMD_ECHO: {
                echo_command(iss);
                break;
            }
            case CMD_TYPE: {
                type_command(iss);
                break;
            }
            case CMD_EXIT_0: {
                if (exit_command(iss) == 0) {
                    return 0;
                }
                std::cout << RED << input << RESET << ": not found" << std::endl;
                break;
            }
            case CMD_PWD: {
                pwd_command(iss);
                break;
            }
            case CMD_CD: {
                cd_command(iss);
            }
            case CMD_UNKNOWN:
            default: {
                // Try to execute the command as an external program
                attempt_exec_command(iss, input, first_word);
                break;
            }
        }
    }
}

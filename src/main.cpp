#include <iostream>
#include <sstream>
#include <string>
#include <cstdlib>
#include <vector>
#include <unistd.h>    // For fork, execvp
#include <sys/wait.h>  // For waitpid

#include "terminal_highlight.h" // Ensure this header is correctly included
#include "commands.h"

// Function to split a string by a delimiter
std::vector<std::string> split(const std::string& str, char delimiter) {
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

// Function to find an executable in the PATH
std::string find_executable(const std::string& command) {
    const char* path_env = std::getenv("PATH");
    if (!path_env) return "";

    std::string path_var(path_env);
    std::vector<std::string> directories = split(path_var, ':');

    for (const auto& dir : directories) {
        std::string full_path = dir + "/" + command;
        if (is_executable(full_path)) {
            return full_path;
        }
    }
    return ""; // Not found
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

        // Check if it's a built-in command
        switch (get_command(first_word)) {
            case CMD_ECHO: {
                std::string remaining;
                std::getline(iss, remaining);
                if (!remaining.empty()) {
                    remaining.erase(0, remaining.find_first_not_of(' '));
                    std::cout << remaining << std::endl;
                } else {
                    std::cout << RED << "echo: missing argument" << RESET << std::endl;
                }
                break;
            }
            case CMD_TYPE: {
                std::string second_word;
                iss >> second_word;

                if (!second_word.empty()) {
                    if (get_command(second_word) != CMD_UNKNOWN) {
                        std::cout << second_word << " is a shell builtin" << std::endl;
                    } else {
                        std::string executable = find_executable(second_word);
                        if (!executable.empty()) {
                            std::cout << second_word << " is " << executable << std::endl;
                        } else {
                            std::cout << RED << second_word << RESET << ": not found" << std::endl;
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
                    return 0;
                } else {
                    std::cout << RED << input << RESET << ": not found" << std::endl;
                }
                break;
            }
            case CMD_UNKNOWN:
            default: {
                // Try to execute the command as an external program
                std::string executable = find_executable(first_word);
                if (!executable.empty()) {
                    // Prepare arguments for execvp
                    std::vector<std::string> args;
                    args.push_back(executable);

                    std::string arg;
                    while (iss >> arg) {
                        args.push_back(arg);
                    }

                    // Convert arguments to char* array
                    std::vector<char*> argv;
                    for (auto& arg : args) {
                        argv.push_back(const_cast<char*>(arg.c_str()));
                    }
                    argv.push_back(nullptr);

                    // Fork and execute
                    pid_t pid = fork();
                    if (pid == 0) {
                        // Child process
                        execvp(argv[0], argv.data());
                        // If execvp fails
                        std::cerr << RED << "Error: Failed to execute " << argv[0] << RESET << std::endl;
                        exit(EXIT_FAILURE);
                    } else if (pid > 0) {
                        // Parent process
                        int status;
                        waitpid(pid, &status, 0);
                    } else {
                        // Fork failed
                        std::cerr << RED << "Error: Failed to fork" << RESET << std::endl;
                    }
                } else {
                    std::cout << RED << input << RESET << ": not found" << std::endl;
                }
                break;
            }
        }
    }
}

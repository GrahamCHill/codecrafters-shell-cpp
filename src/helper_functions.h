//
// Created by Graham Hill on 2025/01/06.
//

#ifndef HELPER_FUNCTIONS_H
#define HELPER_FUNCTIONS_H

#include <cstdlib>
#include <vector>
#include <unistd.h>    // For fork, execvp
#include <sys/wait.h>  // For waitpid
#include <limits.h>

// Function to split a string by a delimiter
inline std::vector<std::string> split(const std::string& str, char delimiter) {
    std::vector<std::string> tokens;
    std::string token;
    std::istringstream tokenStream(str);
    while (std::getline(tokenStream, token, delimiter)) {
        tokens.push_back(token);
    }
    return tokens;
}

// Function to check if a file exists and is executable
inline bool is_executable(const std::string& path) {
    return access(path.c_str(), X_OK) == 0;
}

// Function to find an executable in the PATH
inline std::string find_executable(const std::string& command) {
    const char* path_env = std::getenv("PATH");
    if (!path_env) return "";

    std::string path_var(path_env);
    std::vector<std::string> directories = split(path_var, ':');

    for (const auto& dir : directories) {
        std::string full_path = dir + "/";
        full_path.append(command);
        if (is_executable(full_path)) {
            return full_path;
        }
    }
    return ""; // Not found
}

// Function to handle Printing out what user typed
inline void echo_command(std::istringstream& input_string) {
    std::string remaining;
    std::getline(input_string, remaining); // Read the remaining part of the input
    // Check if there's any remaining input
    if (remaining.empty() || remaining.find_first_not_of(' ') == std::string::npos) {
        std::cout << RED << "echo: missing argument" << RESET << std::endl;
    } else {
        std::cout << remaining << std::endl; // Output the remaining input as is
    }
}

// Function for determining type of second argument (ie echo, exit, etc.)
inline void type_command(std::istringstream& input_string) {
    std::string second_word;
    input_string >> second_word;

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
}

// Function for graceful exit
inline int exit_command(std::istringstream& input_string) {
    std::string argument;
    input_string >> argument;

    if (argument == "0") {
        return 0;
    }
    return 1;
}

inline void attempt_exec_command(std::istringstream &input_string, const std::string &input, const std::string& first_word) {
    if (std::string executable = find_executable(first_word); !executable.empty()) {
        // Prepare arguments for execvp
        std::vector<std::string> args;
        args.push_back(executable);

        std::string arg;
        while (input_string >> arg) {
            args.push_back(arg);
        }

        // Convert arguments to char* array
        std::vector<char*> argv;
        argv.reserve(args.size());
        for (auto& argument : args) {
            argv.push_back(const_cast<char*>(argument.c_str()));
        }
        argv.push_back(nullptr);

        // Fork and execute
        if (pid_t pid = fork(); pid == 0) {
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
}

inline void pwd_command(std::istringstream& input_string) {
    if (char buffer[PATH_MAX]; getcwd(buffer, sizeof(buffer)) != nullptr) {
        std::cout << buffer << std::endl; // Print the current working directory
    } else {
        std::cout << "pwd: error retrieving current directory" << std::endl;
    }
}
#endif //HELPER_FUNCTIONS_H

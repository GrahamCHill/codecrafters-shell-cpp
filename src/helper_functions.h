//
// Created by Graham Hill on 2025/01/06.
//

#ifndef HELPER_FUNCTIONS_H
#define HELPER_FUNCTIONS_H

#include <cstdlib>
#include <vector>
#include <unistd.h>    // For fork, execvp
#include <sys/wait.h>  // For waitpid
#include <climits>
#include <iostream>
#include <filesystem>
#include <string>
#include <sys/stat.h>
#include <fstream>
#include <sstream>

#ifndef PATH_MAX
#define PATH_MAX 1024  // Define a custom value if PATH_MAX is not already defined
#endif

inline std::string currentWorkingDirectory;

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
    // If a current working directory is provided, try searching within it first
    std::string full_path = currentWorkingDirectory + "/" + command;
    if (is_executable(full_path)) {
        return full_path; // Return full path if executable is found in the given directory
    }

    // If not found, check in the directories listed in the PATH environment variable
    const char* path_env = std::getenv("PATH");
    if (!path_env) return "";

    // System Path
    std::string path_var(path_env);

    // Splitting each value in path
    for (const std::vector<std::string> directories = split(path_var, ':');
    // Check each directory in PATH for the executable
        const auto& dir : directories) {
        full_path = dir + "/";
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
    remaining.erase(0, remaining.find_first_not_of(' '));
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
        // Change current working directory to global currentWorkingDirectory
        if (chdir(currentWorkingDirectory.c_str()) != 0) {
            std::cerr << RED << "Error: Failed to change directory to " << currentWorkingDirectory << RESET << std::endl;
            return;
        }

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
    std::cout << currentWorkingDirectory << std::endl; // Print the current working directory
}

inline void set_initial_directory_command() {
    if (char buffer[PATH_MAX]; getcwd(buffer, sizeof(buffer)) != nullptr) {
        currentWorkingDirectory.append(buffer); // Print the current working directory
    }
}

inline std::string get_current_directory_command() {
    return currentWorkingDirectory;
}

inline void set_current_directory_command(const std::string& currentDirectory) {
    currentWorkingDirectory = currentDirectory;
}



inline void cd_command(std::istringstream& input_string) {
    std::string remaining;
    std::getline(input_string, remaining); // Read the remaining part of the input
    remaining.erase(0, remaining.find_first_not_of(' ')); // Remove leading spaces

    if (remaining.empty()) {
        std::cout << RED << "cd: missing argument" << RESET << std::endl;
        return;
    }

    std::string currentDirectory = get_current_directory_command();

    // Check if the path is absolute or relative
    std::string targetDirectory;

    if (remaining[0] == '/') {
        // Absolute path, no need to append to current directory
        const char* home = std::getenv("HOME");
        targetDirectory = home + remaining;
    } else {
        // Relative path, append to the current working directory
        targetDirectory = currentDirectory + "/" + remaining;
    }

    // Normalize the path (resolve relative components like "..")
    std::filesystem::path path(targetDirectory);
    try {
        // Resolve the path
        path = canonical(path);

        // Check if the directory exists
        if (!exists(path) || !is_directory(path)) {
            std::cout << RED << "cd: " << path << ": No such file or directory" << RESET << std::endl;
        } else {
            // Set the new current working directory
            set_current_directory_command(path.string());
        }
    } catch (const std::filesystem::filesystem_error& e) {
        std::cout << RED << "cd: " << e.what() << RESET << std::endl;
    }
    attempt_exec_command(input_string, remaining, "ls");
}
#endif //HELPER_FUNCTIONS_H

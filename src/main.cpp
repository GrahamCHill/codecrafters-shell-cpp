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
Command get_command(const std::string& input) {
  static const std::unordered_map<std::string, Command> command_map = {
    {"echo", CMD_ECHO},
    {"type", CMD_TYPE},
    {"exit 0", CMD_EXIT_0} // Exact match for "exit 0"
  };

  const auto it = command_map.find(input);
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

    // Check command
    switch (get_command(input)) {
      case CMD_ECHO: {
        std::istringstream iss(input);
        std::string first_word;
        iss >> first_word;

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
        std::istringstream iss(input);
        std::string first_word, second_word;
        iss >> first_word >> second_word;

        if (!second_word.empty()) {
          std::cout << second_word << " is a shell builtin" << std::endl;
        } else {
          std::cout << RED << input << RESET << ": command not found" << std::endl;
        }
        break;
      }
      case CMD_EXIT_0:
        return 0; // Exit successfully
      case CMD_UNKNOWN:
      default:
        std::cout << RED << input << RESET << ": command not found" << std::endl;
        break;
    }
  }
}

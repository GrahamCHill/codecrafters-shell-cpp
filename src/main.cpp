#include <iostream>
#include <sstream>

#include "terminal_highlight.h"

int main() {

  // Flush after every std::cout / std:cerr
  std::cout << std::unitbuf;
  std::cerr << std::unitbuf;
  std::cout << "$ ";

  while (true) {
    std::string input;
    std::getline(std::cin, input);
    std::istringstream iss(input);
    std::string first_word;
    iss >> first_word;
    if (input == "exit 0")
      return 0;
    if (first_word == "echo") {
      std::string remaining;
      std::getline(iss, remaining);
      remaining.erase(0, remaining.find_first_not_of(' ')); // Remove leading spaces
      std::cout << remaining << std::endl; // Print the remaining text
    }
    else if (first_word == "type" && iss.peek() != EOF) {
      std::string second_word;
      iss >> second_word;
      std::cout << second_word << " is a shell builtin"<< std::endl;
    }
    else
      std::cout << RED << input << ":"<< RESET << " command not found" << std::endl;


    std::cout << "$ ";

  }

}

#include <iostream>
#include <sstream>

int main() {
  // Flush after every std::cout / std:cerr
  std::cout << std::unitbuf;
  std::cerr << std::unitbuf;

  while (true) {
    std::cout << "$ ";

    std::string input;
    std::getline(std::cin, input);
    std::istringstream iss(input);
    std::string first_word, second_word;
    iss >> first_word >> second_word;
    if (input == "exit 0")
      return 0;
    if (first_word == "echo") {
      std::string remaining;
      std::getline(iss, remaining);
      remaining.erase(0, remaining.find_first_not_of(' '));
      std::cout << remaining << std::endl;
    }
    else if (first_word == "type" && !second_word.empty()) {
      std::cerr << first_word << " is a shell builtin \n";
    }
    else
      std::cerr << input << ": command not found" << std::endl;
  }

}

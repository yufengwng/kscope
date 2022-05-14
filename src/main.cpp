#include "parser.h"
#include <iostream>
#include <sstream>
#include <string>

int main() {
  std::cout << "[kscope]" << std::endl;

  std::string input;
  while (true) {
    std::cerr << "ks> ";
    std::cerr.flush();

    std::getline(std::cin, input);
    if (std::cin.eof()) {
      break;
    }

    std::stringstream src(input);
    kscope::Parser parser(src);
    parser.dispatch();
  }

  return 0;
}

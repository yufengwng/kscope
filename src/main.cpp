#include "parser.h"
#include <iostream>

int main() {
  std::cout << "[kscope]" << std::endl;

  kscope::Parser parser;
  parser.dispatch();

  return 0;
}

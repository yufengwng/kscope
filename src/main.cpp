#include "lexer.h"
#include <cstdio>
#include <iostream>

int main() {
  std::cout << "[kscope]" << std::endl;

  kscope::Lexer lexer;
  int token = lexer.scan_token();
  while (token != EOF) {
    std::cout << "token: " << token;
    if (token == kscope::TK_DEF) {
      std::cout << " (def)";
    } else if (token == kscope::TK_EXTERN) {
      std::cout << " (extern)";
    } else if (token == kscope::TK_IDENT) {
      std::cout << " (" << lexer.get_ident_str() << ")";
    } else if (token == kscope::TK_NUM) {
      std::cout << " (" << lexer.get_num_value() << ")";
    }
    std::cout << std::endl;
    token = lexer.scan_token();
  }
  std::cout << "token: eof" << std::endl;

  return 0;
}

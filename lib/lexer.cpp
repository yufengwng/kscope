#include "lexer.h"
#include <cctype>
#include <cstdio>
#include <cstdlib>

namespace kscope {

int Lexer::scan_token() {
  last_char_ = ' ';

  // Skip whitespace (space, tab, etc).
  while (isblank(last_char_)) {
    last_char_ = getchar();
  }

  // Identifier: [a-zA-Z][a-zA-Z0-9_]*
  if (isalpha(last_char_)) {
    ident_str_ = last_char_;
    while (isalnum((last_char_ = getchar())) || last_char_ == '_') {
      ident_str_ += last_char_;
    }
    if (ident_str_ == "def") {
      return TK_DEF;
    } else if (ident_str_ == "extern") {
      return TK_EXTERN;
    } else {
      return TK_IDENT;
    }
  }

  // Number: [0-9.]+
  if (isdigit(last_char_) || last_char_ == '.') {
    std::string num_str;
    num_str = last_char_;
    while (isdigit((last_char_ = getchar())) || last_char_ == '.') {
      num_str += last_char_;
    }
    num_val_ = strtod(num_str.c_str(), nullptr);
    return TK_NUM;
  }

  // Comment: goes until end of line.
  if (last_char_ == '#') {
    do {
      last_char_ = getchar();
    } while (last_char_ != EOF && last_char_ != '\n' && last_char_ != '\r');
    if (last_char_ != EOF) {
      return scan_token();
    }
  }

  // Check for end-of-file.
  if (last_char_ == EOF) {
    return TK_EOF;
  }

  // Otherwise, return ascii char as-is.
  int prev_char = last_char_;
  last_char_ = getchar();
  return prev_char;
}

std::string& Lexer::get_ident_str() {
  return ident_str_;
}

double Lexer::get_num_value() {
  return num_val_;
}

} // namespace kscope

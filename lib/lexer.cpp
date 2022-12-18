#include "lexer.h"
#include <cctype>

namespace kscope {

Lexer::Lexer(std::istream& src)
    : src_(src) {
  last_char_ = ' ';
}

int Lexer::scan_token() {
  if (src_.eof()) {
    return TK_EOF;
  }

  // Skip whitespace (space, tab, etc).
  while (!src_.eof() && std::isblank(last_char_)) {
    last_char_ = src_.get();
  }

  // Identifier: [a-zA-Z][a-zA-Z0-9_]*
  if (std::isalpha(last_char_)) {
    ident_str_ = "";
    do {
      ident_str_ += last_char_;
      last_char_ = src_.get();
    } while (!src_.eof() && (std::isalnum(last_char_) || last_char_ == '_'));
    if (ident_str_ == "def") {
      return TK_DEF;
    } else if (ident_str_ == "extern") {
      return TK_EXTERN;
    } else if (ident_str_ == "if") {
      return TK_IF;
    } else if (ident_str_ == "else") {
      return TK_ELSE;
    } else {
      return TK_IDENT;
    }
  }

  // Number: [0-9.]+
  if (std::isdigit(last_char_) || last_char_ == '.') {
    std::string num_str;
    do {
      num_str += last_char_;
      last_char_ = src_.get();
    } while (!src_.eof() && (std::isdigit(last_char_) || last_char_ == '.'));
    num_val_ = std::stod(num_str);
    return TK_NUM;
  }

  // Comment: goes until end of line.
  if (last_char_ == '#') {
    do {
      last_char_ = src_.get();
    } while (!src_.eof() && last_char_ != '\n' && last_char_ != '\r');
    if (!src_.eof()) {
      return scan_token();
    }
  }

  // Check for end-of-file.
  if (src_.eof()) {
    return TK_EOF;
  }

  // Otherwise, return ascii char as-is.
  int prev_char = last_char_;
  last_char_ = src_.get();
  return prev_char;
}

const std::string& Lexer::get_ident_str() const {
  return ident_str_;
}

double Lexer::get_num_value() const {
  return num_val_;
}

} // namespace kscope

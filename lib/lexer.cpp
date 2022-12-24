#include "lexer.h"
#include <cctype>

namespace kscope {

Lexer::Lexer(std::istream& src)
    : src_(src) {
  last_char_ = ' ';
}

int Lexer::scan_token() {
  if (is_eof()) {
    return TK_EOF;
  }

  // Skip whitespace (space, tab, etc).
  while (!is_eof() && std::isblank(last_char_)) {
    last_char_ = next_char();
  }

  // Operators.
  if (last_char_ == '.') {
    // Look for .. (range, dot-dot).
    if (peek_char() == '.') {
      next_char(); // Consume '..'.
      last_char_ = next_char();
      return TK_DOT2;
    }
  }

  // Identifier: [a-zA-Z][a-zA-Z0-9_]*
  if (std::isalpha(last_char_)) {
    ident_str_ = "";
    do {
      ident_str_ += last_char_;
      last_char_ = next_char();
    } while (!is_eof() && (std::isalnum(last_char_) || last_char_ == '_'));
    if (ident_str_ == "def") {
      return TK_DEF;
    } else if (ident_str_ == "extern") {
      return TK_EXTERN;
    } else if (ident_str_ == "if") {
      return TK_IF;
    } else if (ident_str_ == "else") {
      return TK_ELSE;
    } else if (ident_str_ == "for") {
      return TK_FOR;
    } else if (ident_str_ == "in") {
      return TK_IN;
    } else {
      return TK_IDENT;
    }
  }

  // Number: [0-9]+ ('.' [0-9]+)?
  if (std::isdigit(last_char_)) {
    std::string num_str;
    do {
      num_str += last_char_;
      last_char_ = next_char();
    } while (!is_eof() && std::isdigit(last_char_));
    // Optional fraction part.
    if (last_char_ == '.' && std::isdigit(peek_char())) {
       do {
         num_str += last_char_;
         last_char_ = next_char();
       } while (!is_eof() && std::isdigit(last_char_));
    }
    num_val_ = std::stod(num_str);
    return TK_NUM;
  }

  // Comment: goes until end of line.
  if (last_char_ == '#') {
    do {
      last_char_ = next_char();
    } while (!is_eof() && last_char_ != '\n' && last_char_ != '\r');
    if (!is_eof()) {
      return scan_token();
    }
  }

  // Check for end-of-file.
  if (is_eof()) {
    return TK_EOF;
  }

  // Otherwise, return ascii char as-is.
  int prev_char = last_char_;
  last_char_ = next_char();
  return prev_char;
}

bool Lexer::is_eof() {
  if (lookahead_.has_value()) {
    return false;
  } else {
    return src_.eof();
  }
}

int Lexer::next_char() {
  if (lookahead_.has_value()) {
    int taken = lookahead_.value();
    lookahead_.reset();
    return taken;
  } else {
    return src_.get();
  }
}

int Lexer::peek_char() {
  if (lookahead_.has_value()) {
    return lookahead_.value();
  }
  char peek = src_.get();
  if (!src_.eof()) {
    lookahead_ = peek;
    return peek;
  } else {
    return TK_EOF;
  }
}

const std::string& Lexer::get_ident_str() const {
  return ident_str_;
}

double Lexer::get_num_value() const {
  return num_val_;
}

} // namespace kscope

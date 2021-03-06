#pragma once

#include "common.h"
#include <iostream>

namespace kscope {

/// Represents the known lexical tokens in language. Lexer returns [0-255] if
/// it is an unknown character.
enum Token {
  TK_EOF = -1,

  // Commands
  TK_DEF = -2,
  TK_EXTERN = -3,

  // Primary
  TK_IDENT = -4,
  TK_NUM = -5,
};

class Lexer {
public:
  Lexer(std::istream& src);

  /// Returns next token from standard input.
  int scan_token();

  /// Returns lexeme if token is identifier.
  const std::string& get_ident_str() const;

  /// Returns numeric value if token is number.
  double get_num_value() const;

private:
  std::istream& src_;
  int last_char_;
  std::string ident_str_;
  double num_val_;
};

} // namespace kscope

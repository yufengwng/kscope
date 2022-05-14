#pragma once

#include <string>

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
  /// Returns next token from standard input.
  int scan_token();

  /// Returns lexeme if token is identifier.
  std::string& get_ident_str();

  /// Returns numeric value if token is number.
  double get_num_value();

private:
  int last_char_;
  std::string ident_str_;
  double num_val_;
};

} // namespace kscope

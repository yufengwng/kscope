#pragma once

#include "common.h"
#include <iostream>
#include <optional>

namespace kscope {

/// Represents the known lexical tokens in language. Lexer returns [0-255] if
/// it is not one of these specified tokens.
enum Token {
  TK_EOF = -1,

  // Commands
  TK_DEF = -11,
  TK_EXTERN = -12,

  // Keywords
  TK_IF = -21,
  TK_ELSE = -22,

  // Primary
  TK_IDENT = -51,
  TK_NUM = -52,
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
  std::optional<int> lookahead_;

  bool is_eof();
  int next_char();
  int peek_char();
};

} // namespace kscope

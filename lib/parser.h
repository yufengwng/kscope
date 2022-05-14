#pragma once

#include "common.h"
#include "ast.h"
#include "lexer.h"

namespace kscope {

class Parser {
public:
  Parser(std::istream& src);

  void dispatch();

private:
  Lexer lexer_;
  int cur_tok_;

  /// Reads another token from lexer and updates `cur_tok`.
  int next_token();

  /// Get precedence of pending binary operator token.
  int get_bin_precedence();

  /// top_level_expr ::= expr
  Box<FunctionAST> parse_top_level_expr();
  /// external ::= 'extern' prototype
  Box<PrototypeAST> parse_extern();
  /// definition ::= 'def' prototype expr
  Box<FunctionAST> parse_definition();
  /// prototype ::= ident '(' ident* ')'
  Box<PrototypeAST> parse_prototype();
  /// expr ::= primary bin_rhs
  Box<ExprAST> parse_expr();
  /// bin_rhs ::= (OP primary)*
  Box<ExprAST> parse_bin_rhs(int prec, Box<ExprAST> lhs);
  /// primary ::= ident_expr | num_expr | paren_expr
  Box<ExprAST> parse_primary();
  /// ident_expr ::= ident | ident '(' expr* ')'
  Box<ExprAST> parse_ident_expr();
  /// num_expr ::= number
  Box<ExprAST> parse_num_expr();
  /// paren_expr ::= '(' expr ')'
  Box<ExprAST> parse_paren_expr();
};

} // namespace kscope

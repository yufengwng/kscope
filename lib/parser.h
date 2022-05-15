#pragma once

#include "ast.h"
#include "lexer.h"
#include <iostream>

namespace kscope {

class Parser {
public:
  Parser(std::istream& src);

  /// top ::= definition | external | expr | ';'
  std::vector<Box<ItemAST>> parse();

  bool errored() const {
    return errored_;
  }

private:
  Lexer lexer_;
  int cur_tok_;
  bool errored_;

  /// Reads another token from lexer and updates `cur_tok`.
  int next_token();

  /// Get precedence of pending binary operator token.
  int get_bin_precedence();

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

  /// Helper for error handling.
  Box<ExprAST> log_err(llvm::StringRef msg);
  /// Helper for error handling typed to prototypes.
  Box<PrototypeAST> log_err_proto(llvm::StringRef msg);
};

} // namespace kscope

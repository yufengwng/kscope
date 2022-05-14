#include "parser.h"
#include <cctype>
#include <iostream>

namespace kscope {

/// Helper for error handling.
Box<ExprAST> log_err(const char* msg) {
  std::cerr << "[error] " << msg << std::endl;
  return nullptr;
}

/// Helper for error handling typed to prototypes.
Box<PrototypeAST> log_err_proto(const char* msg) {
  log_err(msg);
  return nullptr;
}

void Parser::dispatch() {
  // Prime the first token.
  std::cerr << "kscope> ";
  next_token();

  while (true) {
    std::cerr << "kscope> ";
    switch (cur_tok_) {
    case TK_EOF:
      return;
    case ';':
      next_token();  // Ignore top-level semicolons.
      break;
    case TK_DEF:
      if (parse_definition()) {
        std::cerr << "parsed function definition" << std::endl;
      } else {
        next_token();  // Skip token for error recovery.
      }
      break;
    case TK_EXTERN:
      if (parse_extern()) {
        std::cerr << "parsed an extern" << std::endl;
      } else {
        next_token();  // Skip token for error recovery.
      }
      break;
    default:
      if (parse_top_level_expr()) {
        std::cerr << "parsed top-level expression" << std::endl;
      } else {
        next_token();  // Skip token for error recovery.
      }
      break;
    }
  }  
}

int Parser::next_token() {
  cur_tok_ = lexer_.scan_token();
  return cur_tok_;
}

int Parser::get_bin_precedence() {
  if (!isascii(cur_tok_)) {
    return -1;
  }
  switch (cur_tok_) {
  case '<': return 10;  // lowest
  case '+': return 20;
  case '-': return 20;
  case '*': return 40;  // highest
  default:  return -1;
  }
}

Box<FunctionAST> Parser::parse_top_level_expr() {
  if (auto expr = parse_expr()) {
    auto anon_proto = std::make_unique<PrototypeAST>("", std::vector<std::string>());
    return std::make_unique<FunctionAST>(std::move(anon_proto), std::move(expr));
  }
  return nullptr;
}

Box<PrototypeAST> Parser::parse_extern() {
  next_token();  // Consume 'extern'.
  return parse_prototype();
}

Box<FunctionAST> Parser::parse_definition() {
  next_token();  // Consume 'def'.
  auto proto = parse_prototype();
  if (!proto) {
    return nullptr;
  }
  if (auto expr = parse_expr()) {
    return std::make_unique<FunctionAST>(std::move(proto), std::move(expr));
  }
  return nullptr;
}

Box<PrototypeAST> Parser::parse_prototype() {
  if (cur_tok_ != TK_IDENT) {
    return log_err_proto("expected function name in prototype");
  }

  std::string name = lexer_.get_ident_str();
  next_token();  // Consume ident.
  if (cur_tok_ != '(') {
    return log_err_proto("expected '(' in prototype");
  }

  std::vector<std::string> params;
  while (next_token() == TK_IDENT) {
    params.push_back(lexer_.get_ident_str());
  }

  if (cur_tok_ != ')') {
    return log_err_proto("expected ')' in prototype");
  }
  next_token();  // Consume ')'.

  return std::make_unique<PrototypeAST>(name, std::move(params));
}

Box<ExprAST> Parser::parse_expr() {
  auto lhs = parse_primary();
  if (!lhs) {
    return nullptr;
  }
  return parse_bin_rhs(0, std::move(lhs));
}

Box<ExprAST> Parser::parse_bin_rhs(int prec, Box<ExprAST> lhs) {
  while (true) {
    // Proceed if binop binds as tighly as current precedence, otherwise return.
    int tok_prec = get_bin_precedence();
    if (tok_prec < prec) {
      return lhs;
    }

    int bin_op = cur_tok_;
    next_token();  // Consume the operator.
    auto rhs = parse_primary();
    if (!rhs) {
      return nullptr;
    }

    // Proceed if binop binds more tighly than next operator, otherwise let
    // pending operator take RHS as its LHS.
    int next_prec = get_bin_precedence();
    if (tok_prec < next_prec) {
      rhs = parse_bin_rhs(tok_prec + 1, std::move(rhs));
      if (!rhs) {
        return nullptr;
      }
    }

    // Merge into binary expression and repeat.
    lhs = std::make_unique<BinExprAST>(bin_op, std::move(lhs), std::move(rhs));
  }
}

Box<ExprAST> Parser::parse_primary() {
  switch (cur_tok_) {
  case TK_IDENT:
    return parse_ident_expr();
  case TK_NUM:
    return parse_num_expr();
  case '(':
    return parse_paren_expr();
  default:
    return log_err("unknown token when expecting an expression");
  }
}

Box<ExprAST> Parser::parse_ident_expr() {
  std::string name = lexer_.get_ident_str();
  next_token();  // Consume ident.

  // A simple variable reference.
  if (cur_tok_ != '(') {
    return std::make_unique<VarExprAST>(name);
  }

  // Else, a function call.
  next_token();  // Consume '('.
  std::vector<Box<ExprAST>> args;
  if (cur_tok_ != ')') {
    while (true) {
      if (auto arg = parse_expr()) {
        args.push_back(std::move(arg));
      } else {
        return nullptr;
      }
      if (cur_tok_ == ')') {
        break;
      }
      if (cur_tok_ != ',') {
        return log_err("expected ')' or ',' in argument list");
      }
      next_token();
    }
  }
  next_token();  // Consume ')'.

  return std::make_unique<CallExprAST>(name, std::move(args));
}

Box<ExprAST> Parser::parse_num_expr() {
  auto res = std::make_unique<NumExprAST>(lexer_.get_num_value());
  next_token();  // Consume the token.
  return std::move(res);
}

Box<ExprAST> Parser::parse_paren_expr() {
  next_token();  // Consume '('.
  auto expr = parse_expr();
  if (!expr) {
    return nullptr;
  }
  if (cur_tok_ != ')') {
    return log_err("expected ')'");
  }
  next_token();  // Consume ')'.
  return expr;
}

} // namespace kscope

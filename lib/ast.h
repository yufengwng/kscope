#pragma once

#include "common.h"

namespace kscope {

/// Base class for all expression nodes.
class ExprAST {
public:
  virtual ~ExprAST() {}
};

/// Represents numeric literals.
class NumExprAST : public ExprAST {
public:
  NumExprAST(double val);

private:
  double val_;
};

/// Represents a variable reference.
class VarExprAST : public ExprAST {
public:
  VarExprAST(const std::string& name);

private:
  std::string name_;
};

/// Represents a binary operator.
class BinExprAST : public ExprAST {
public:
  BinExprAST(char op, Box<ExprAST> lhs, Box<ExprAST> rhs);

private:
  char op_;
  Box<ExprAST> lhs_, rhs_;
};

/// Represents a function call.
class CallExprAST : public ExprAST {
public:
  CallExprAST(const std::string& callee, std::vector<Box<ExprAST>> args);

private:
  std::string callee_;
  std::vector<Box<ExprAST>> args_;
};

/// Represents a function signature, which captures its name and arguments.
class PrototypeAST {
public:
  PrototypeAST(const std::string& name, std::vector<std::string> args);

  /// Returns the function name.
  const std::string& get_name() const;

private:
  std::string name_;
  std::vector<std::string> args_;
};

/// Represents a function definition.
class FunctionAST {
public:
  FunctionAST(Box<PrototypeAST> proto, Box<ExprAST> body);

private:
  Box<PrototypeAST> proto_;
  Box<ExprAST> body_;
};

} // namespace kscope

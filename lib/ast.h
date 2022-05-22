#pragma once

#include "common.h"

namespace kscope {

class ExprAST;

/// Base class for all items in AST.
class ItemAST {
public:
  enum ItemKind {
    IK_PROTO,
    IK_FUNC,
    IK_EXPR,
    IK_NUM_EXPR,
    IK_VAR_EXPR,
    IK_BIN_EXPR,
    IK_CALL_EXPR,
    IK_LAST_EXPR,
  };

  virtual ~ItemAST() = default;

  ItemKind kind() const {
    return kind_;
  }

protected:
  const ItemKind kind_;

  ItemAST(ItemKind kind) : kind_(kind) {}
};

/// A function signature, which captures its name and arguments.
class PrototypeAST : public ItemAST {
public:
  static bool classof(const ItemAST* item) {
    return item->kind() == IK_PROTO;
  }

  PrototypeAST(const std::string& name, std::vector<std::string> args);

  const std::string& name() const {
    return name_;
  }

  const std::vector<std::string>& args() const {
    return args_;
  }

  size_t num_args() const {
    return args_.size();
  }

private:
  std::string name_;
  std::vector<std::string> args_;
};

/// A function definition, with its prototype and body.
class FunctionAST : public ItemAST {
public:
  static std::string ANON_NAME;

  static bool classof(const ItemAST* item) {
    return item->kind() == IK_FUNC;
  }

  /// Wrap the expression in an anonymous function definition.
  static Box<FunctionAST> make_anon(Box<ExprAST> expr);

  FunctionAST(Box<PrototypeAST> proto, Box<ExprAST> body);

  Box<PrototypeAST> copy_proto() const;

  const PrototypeAST* proto() const {
    return proto_.get();
  }

  const ExprAST* body() const {
    return body_.get();
  }

private:
  Box<PrototypeAST> proto_;
  Box<ExprAST> body_;
};

/// Base class for all expression nodes.
class ExprAST : public ItemAST {
public:
  static bool classof(const ItemAST* item) {
    return IK_EXPR <= item->kind() && item->kind() <= IK_LAST_EXPR;
  }

  virtual ~ExprAST() = default;

protected:
  ExprAST(ItemKind expr_kind) : ItemAST(expr_kind) {}
};

/// Represents numeric literals.
class NumExprAST : public ExprAST {
public:
  static bool classof(const ItemAST* item) {
    return item->kind() == IK_NUM_EXPR;
  }

  NumExprAST(double val);

  double value() const {
    return val_;
  }

private:
  double val_;
};

/// Represents a variable reference.
class VarExprAST : public ExprAST {
public:
  static bool classof(const ItemAST* item) {
    return item->kind() == IK_VAR_EXPR;
  }

  VarExprAST(const std::string& name);

  const std::string& name() const {
    return name_;
  }

private:
  std::string name_;
};

/// Represents a binary operator.
class BinExprAST : public ExprAST {
public:
  static bool classof(const ItemAST* item) {
    return item->kind() == IK_BIN_EXPR;
  }

  BinExprAST(char op, Box<ExprAST> lhs, Box<ExprAST> rhs);

  char op() const {
    return op_;
  }

  const ExprAST* lhs() const {
    return lhs_.get();
  }

  const ExprAST* rhs() const {
    return rhs_.get();
  }

private:
  char op_;
  Box<ExprAST> lhs_, rhs_;
};

/// Represents a function call.
class CallExprAST : public ExprAST {
public:
  static bool classof(const ItemAST* item) {
    return item->kind() == IK_CALL_EXPR;
  }

  CallExprAST(const std::string& callee, std::vector<Box<ExprAST>> args);

  const std::string& callee() const {
    return callee_;
  }

  const std::vector<Box<ExprAST>>& args() const {
    return args_;
  }

  size_t num_args() const {
    return args_.size();
  }

private:
  std::string callee_;
  std::vector<Box<ExprAST>> args_;
};

} // namespace kscope

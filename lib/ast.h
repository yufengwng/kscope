#pragma once

#include "common.h"

namespace kscope {

class ItemAST;
class ExprAST;

/// Root of AST, which may contain any number of items.
class RootAST {
public:
  RootAST(std::vector<Box<ItemAST>> items);

  const std::vector<Box<ItemAST>>& items() const {
    return items_;
  }

private:
  std::vector<Box<ItemAST>> items_;
};

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

  virtual ~ItemAST() {}

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

private:
  std::string name_;
  std::vector<std::string> args_;
};

/// A function definition, with its prototype and body.
class FunctionAST : public ItemAST {
public:
  static bool classof(const ItemAST* item) {
    return item->kind() == IK_FUNC;
  }

  FunctionAST(Box<PrototypeAST> proto, Box<ExprAST> body);

  const std::string& name() const {
    return proto_->name();
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

  virtual ~ExprAST() {}

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

  size_t num_args() const {
    return args_.size();
  }

private:
  std::string callee_;
  std::vector<Box<ExprAST>> args_;
};

} // namespace kscope

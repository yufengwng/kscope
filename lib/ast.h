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
    IK_EXPR_,
    IK_IF_EXPR,
    IK_FOR_EXPR,
    IK_NUM_EXPR,
    IK_VAR_EXPR,
    IK_BIN_EXPR,
    IK_CALL_EXPR,
    IK_LAST_EXPR_,
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
  inline static std::string ANON_NAME = "__anon__";

  static bool classof(const ItemAST* item) {
    return item->kind() == IK_FUNC;
  }

  /// Wrap the expression in an anonymous function definition.
  static Box<FunctionAST> make_anon(Box<ExprAST> expr);

  FunctionAST(Box<PrototypeAST> proto, Box<ExprAST> body);

  Box<PrototypeAST> clone_proto() const;

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
    return IK_EXPR_ < item->kind() && item->kind() < IK_LAST_EXPR_;
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

/// Represents an if/else expression.
class IfExprAST : public ExprAST {
public:
  static bool classof(const ItemAST* item) {
    return item->kind() == IK_IF_EXPR;
  }

  IfExprAST(Box<ExprAST> cond, Box<ExprAST> then_case, Box<ExprAST> else_case);

  const ExprAST* cond_expr() const {
    return cond_.get();
  }

  const ExprAST* then_expr() const {
    return then_.get();
  }

  const ExprAST* else_expr() const {
    return else_.get();
  }

private:
  Box<ExprAST> cond_, then_, else_;
};

/// Represents a for/in loop expression.
class ForExprAST : public ExprAST {
public:
  inline static double DEFAULT_STEP = 1.0;

  static bool classof(const ItemAST* item) {
    return item->kind() == IK_FOR_EXPR;
  }

  ForExprAST(const std::string& itervar_name,
             Box<ExprAST> init_val, Box<ExprAST> stop_val,
             Box<ExprAST> body_expr, Box<ExprAST> step_val = nullptr);

  const std::string& itervar() const {
    return itervar_;
  }

  const ExprAST* init_expr() const {
    return init_.get();
  }

  const ExprAST* stop_expr() const {
    return stop_.get();
  }

  const ExprAST* body_expr() const {
    return body_.get();
  }

  bool has_step() const {
    return step_ != nullptr;
  }

  const ExprAST* step_expr() const {
    return step_ ? step_.get() : nullptr;
  }

private:
  std::string itervar_;
  Box<ExprAST> init_, stop_, body_;
  Box<ExprAST> step_;  // Optional.
};

} // namespace kscope

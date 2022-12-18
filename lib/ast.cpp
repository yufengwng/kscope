#include "ast.h"

namespace kscope {

PrototypeAST::PrototypeAST(const std::string& name, std::vector<std::string> args)
    : ItemAST(IK_PROTO), name_(name), args_(std::move(args)) {}

FunctionAST::FunctionAST(Box<PrototypeAST> proto, Box<ExprAST> body)
    : ItemAST(IK_FUNC), proto_(std::move(proto)), body_(std::move(body)) {}

Box<FunctionAST> FunctionAST::make_anon(Box<ExprAST> expr) {
  std::vector<std::string> empty;
  auto anon_proto = std::make_unique<PrototypeAST>(ANON_NAME, empty);
  return std::make_unique<FunctionAST>(std::move(anon_proto), std::move(expr));
}

Box<PrototypeAST> FunctionAST::copy_proto() const {
  std::vector<std::string> args(proto_->args());
  return std::make_unique<PrototypeAST>(proto_->name(), args);
}

NumExprAST::NumExprAST(double val)
    : ExprAST(IK_NUM_EXPR), val_(val) {}

VarExprAST::VarExprAST(const std::string& name)
    : ExprAST(IK_VAR_EXPR), name_(name) {}

BinExprAST::BinExprAST(char op, Box<ExprAST> lhs, Box<ExprAST> rhs)
    : ExprAST(IK_BIN_EXPR), op_(op), lhs_(std::move(lhs)), rhs_(std::move(rhs)) {}

CallExprAST::CallExprAST(const std::string& callee, std::vector<Box<ExprAST>> args)
    : ExprAST(IK_CALL_EXPR), callee_(callee), args_(std::move(args)) {}

IfExprAST::IfExprAST(Box<ExprAST> cond, Box<ExprAST> then_case, Box<ExprAST> else_case)
    : ExprAST(IK_IF_EXPR),
      cond_(std::move(cond)),
      then_(std::move(then_case)),
      else_(std::move(else_case)) {}

} // namespace kscope

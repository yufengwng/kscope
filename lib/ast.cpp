#include "ast.h"

namespace kscope {

PrototypeAST::PrototypeAST(const std::string& name, std::vector<std::string> args)
    : ItemAST(IK_PROTO), name_(name), args_(std::move(args)) {}

FunctionAST::FunctionAST(Box<PrototypeAST> proto, Box<ExprAST> body)
    : ItemAST(IK_FUNC), proto_(std::move(proto)), body_(std::move(body)) {}

Box<FunctionAST> FunctionAST::make_anon(Box<ExprAST> expr) {
  std::vector<std::string> empty;
  auto anon_proto = std::make_unique<PrototypeAST>("__anon__", empty);
  return std::make_unique<FunctionAST>(std::move(anon_proto), std::move(expr));
}

NumExprAST::NumExprAST(double val)
    : ExprAST(IK_NUM_EXPR), val_(val) {}

VarExprAST::VarExprAST(const std::string& name)
    : ExprAST(IK_VAR_EXPR), name_(name) {}

BinExprAST::BinExprAST(char op, Box<ExprAST> lhs, Box<ExprAST> rhs)
    : ExprAST(IK_BIN_EXPR), op_(op), lhs_(std::move(lhs)), rhs_(std::move(rhs)) {}

CallExprAST::CallExprAST(const std::string& callee, std::vector<Box<ExprAST>> args)
    : ExprAST(IK_CALL_EXPR), callee_(callee), args_(std::move(args)) {}

} // namespace kscope

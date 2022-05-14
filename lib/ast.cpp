#include "ast.h"

namespace kscope {

NumExprAST::NumExprAST(double val) : val_(val) {}

VarExprAST::VarExprAST(const std::string& name) : name_(name) {}

BinExprAST::BinExprAST(char op, Box<ExprAST> lhs, Box<ExprAST> rhs)
  : op_(op), lhs_(std::move(lhs)), rhs_(std::move(rhs)) {}

CallExprAST::CallExprAST(const std::string& callee, std::vector<Box<ExprAST>> args)
  : callee_(callee), args_(std::move(args)) {}

PrototypeAST::PrototypeAST(const std::string& name, std::vector<std::string> args)
  : name_(name), args_(std::move(args)) {}

const std::string& PrototypeAST::get_name() const {
  return name_;
}

FunctionAST::FunctionAST(Box<PrototypeAST> proto, Box<ExprAST> body)
  : proto_(std::move(proto)), body_(std::move(body)) {}

} // namespace kscope

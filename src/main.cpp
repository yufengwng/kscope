#include "parser.h"
#include <iostream>
#include <sstream>
#include <string>

using namespace kscope;
using namespace llvm;

int main() {
  std::cout << "[kscope]" << std::endl;

  std::string input;
  while (true) {
    std::cerr << "ks> ";
    std::cerr.flush();

    std::getline(std::cin, input);
    if (std::cin.eof()) {
      break;
    }

    std::stringstream src(input);
    Parser parser(src);
    auto ast = parser.parse();

    for (auto& item : ast.items()) {
      auto ptr = item.get();
      if (auto proto = dyn_cast<PrototypeAST>(ptr)) {
        std::cerr << "parsed extern prototype: " << proto->name() << std::endl;
      } else if (auto fn = dyn_cast<FunctionAST>(ptr)) {
        std::cerr << "parsed function definition: " << fn->name() << std::endl;
      } else if (auto expr = dyn_cast<ExprAST>(ptr)) {
        std::cerr << "parsed top-level expression: ";
        if (auto num_expr = dyn_cast<NumExprAST>(expr)) {
          std::cerr << "num (" << num_expr->value() << ")\n";
        } else if (auto var_expr = dyn_cast<VarExprAST>(expr)) {
          std::cerr << "var (" << var_expr->name() << ")\n";
        } else if (auto bin_expr = dyn_cast<BinExprAST>(expr)) {
          std::cerr << "bin (" << bin_expr->op() << ")\n";
        } else if (auto call_expr = dyn_cast<CallExprAST>(expr)) {
          std::cerr << "call (" << call_expr->callee() << ", " << call_expr->num_args() << ")\n";
        } else {
          std::cerr << "unknown\n";
        }
      } else {
        std::cerr << "unknown item\n";
      }
    }

    if (parser.errored()) {
      std::cerr << "note: there were some parse errors\n";
    }
  }

  return 0;
}

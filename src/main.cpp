#include "emitter.h"
#include "parser.h"
#include "runtime.h"
#include "llvm/Support/raw_ostream.h"
#include <iostream>
#include <sstream>
#include <string>

using namespace kscope;
using namespace llvm;

int main() {
  std::cout << "[kscope]" << std::endl;
  Emitter emitter("__main__");
  Optimizer opt(emitter.mod());

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
    auto items = parser.parse();

    for (auto& iter : items) {
      auto item = std::move(iter);
      Function* fn_ir = nullptr;
      if (auto* proto = dyn_cast<PrototypeAST>(item.get())) {
        if ((fn_ir = emitter.codegen(proto))) {
          std::cerr << "read extern prototype:\n";
        }
      } else if (auto* fn = dyn_cast<FunctionAST>(item.get())) {
        if ((fn_ir = emitter.codegen(fn))) {
          std::cerr << "read function definition:\n";
        }
      } else if (auto* expr = dyn_cast<ExprAST>(item.get())) {
        Box<ExprAST> expr_item((ExprAST*) item.release());
        auto anon_fn = FunctionAST::make_anon(std::move(expr_item));
        if ((fn_ir = emitter.codegen(anon_fn.get()))) {
          std::cerr << "read top-level expression:\n";
        }
      } else {
        std::cerr << "unknown item\n";
      }

      if (fn_ir) {
        std::cerr << "  optimizing function: " << fn_ir->getName().str() << std::endl;
        opt.run(fn_ir);
        std::cerr << "  finished optimizing function\n";
        fn_ir->print(errs());
        if (fn_ir->getName() == "__anon__") {
          fn_ir->eraseFromParent();
        }
      }
    }

    if (parser.errored()) {
      std::cerr << "note: there were some parse errors\n";
    }
  }

  std::cerr << "\n=== module ===\n";
  emitter.mod()->print(errs(), nullptr);
  std::cerr << "==============\n";
  if (emitter.errored()) {
    std::cerr << "note: there were some codegen errors\n";
  }

  return 0;
}

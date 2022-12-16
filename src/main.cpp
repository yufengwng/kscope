#include "emitter.h"
#include "executor.h"
#include "parser.h"
#include "llvm/Support/raw_ostream.h"
#include <iostream>
#include <sstream>
#include <string>

using namespace kscope;

namespace {

class Driver {
public:
  Driver() {
    jit_ = Executor::create();
    emitter_ = std::make_unique<Emitter>("__main__", jit_->data_layout());
  }

  ~Driver() {
    trackers_.clear();
  }

  Box<llvm::Module> run() {
    std::cout << "[kscope]" << std::endl;
    while (true) {
      std::cerr << "ks> ";
      std::cerr.flush();

      std::getline(std::cin, input_);
      if (std::cin.eof()) {
        break;
      }

      std::stringstream src(input_);
      Parser parser(src);
      auto items = parser.parse();
      if (parser.errored()) {
        std::cerr << "note: there were some parse errors" << std::endl;
      }

      for (auto& iter : items) {
        auto item = std::move(iter);
        if (llvm::dyn_cast<PrototypeAST>(item.get())) {
          Box<PrototypeAST> proto((PrototypeAST*) item.release());
          handle_extern(std::move(proto));
        } else if (llvm::dyn_cast<FunctionAST>(item.get())) {
          Box<FunctionAST> def((FunctionAST*) item.release());
          handle_define(std::move(def));
        } else if (llvm::dyn_cast<ExprAST>(item.get())) {
          Box<ExprAST> expr((ExprAST*) item.release());
          auto anon_fn = FunctionAST::make_anon(std::move(expr));
          handle_top_level_expr(std::move(anon_fn));
        } else {
          std::cerr << "unknown item" << std::endl;
        }
        std::cerr << std::endl;
      }
    }
    return emitter_->take_mod();
  }

  void handle_extern(Box<PrototypeAST> proto) {
    auto* fn_ir = emitter_->codegen(proto.get());
    if (fn_ir != nullptr && !emitter_->errored()) {
      std::cerr << "read extern prototype:\n";
      fn_ir->print(llvm::errs());
      emitter_->register_proto(std::move(proto));
    } else {
      std::cerr << "note: error during codegen of prototype" << std::endl;
    }
  }

  void handle_define(Box<FunctionAST> def) {
    auto* fn_ir = emitter_->codegen(def.get());
    if (fn_ir != nullptr && !emitter_->errored()) {
      std::cerr << "read function definition:\n";
      fn_ir->print(llvm::errs());

      auto fn_name = fn_ir->getName().str();
      auto iter = trackers_.find(fn_name);
      if (iter != trackers_.end()) {
        jit_->remove_module(iter->second);
      }

      auto mod = emitter_->take_mod();
      auto tracker = jit_->add_module(std::move(mod));
      trackers_[fn_name] = tracker;
    } else {
      std::cerr << "note: error during codegen of function" << std::endl;
    }
  }

  void handle_top_level_expr(Box<FunctionAST> anon_fn) {
    auto* fn_ir = emitter_->codegen(anon_fn.get());
    if (fn_ir != nullptr && !emitter_->errored()) {
      std::cerr << "read top-level expression:\n";
      fn_ir->print(llvm::errs());

      // JIT the module containing the anon function.
      auto mod = emitter_->take_mod();
      auto tracker = jit_->add_module(std::move(mod));

      // Get the symbol address, cast to native function, and call it.
      auto addr = jit_->lookup(FunctionAST::ANON_NAME);
      assert(addr && "anon function not found");

      double (*fp)() = addr->toPtr<double()>();
      double res = fp();
      std::cerr << "evaluated to: " << res << std::endl;

      // Delete the anon module from the JIT.
      jit_->remove_module(tracker);
    } else {
      std::cerr << "note: error during codegen of expression" << std::endl;
    }
  }

private:
  std::map<std::string, llvm::orc::ResourceTrackerSP> trackers_;
  Box<Emitter> emitter_;
  Box<Executor> jit_;
  std::string input_;
};

} // namespace

int main() {
  Executor::init_native_target();
  Driver repl;

  auto mod = repl.run();
  std::cerr << "\n=== module ===\n";
  mod->print(llvm::errs(), nullptr);
  std::cerr << "==============\n";

  return 0;
}

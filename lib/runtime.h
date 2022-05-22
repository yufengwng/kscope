#pragma once

#include "common.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/LegacyPassManager.h"

namespace kscope {

class Optimizer {
public:
  Optimizer(llvm::Module* mod);

  /// Optimize the given function.
  void run(llvm::Function* fn);

private:
  Box<llvm::legacy::FunctionPassManager> fpm_;
};

} // namespace kscope

#include "runtime.h"
#include "llvm/Transforms/InstCombine/InstCombine.h"
#include "llvm/Transforms/Scalar.h"
#include "llvm/Transforms/Scalar/GVN.h"
#include <iostream>

using namespace llvm;

namespace kscope {

Optimizer::Optimizer(Module* mod) {
  fpm_ = std::make_unique<legacy::FunctionPassManager>(mod);
  // Simple peephole and bit-twiddling optimizations.
  fpm_->add(createInstructionCombiningPass());
  // Reassociate expressions into a more canonical form.
  fpm_->add(createReassociatePass());
  // Elimiate redundant expressions.
  fpm_->add(createGVNPass());
  // Simplify control flow graph.
  fpm_->add(createCFGSimplificationPass());
  fpm_->doInitialization();
}

void Optimizer::run(Function* fn) {
  fpm_->run(*fn);
}

} // namespace kscope

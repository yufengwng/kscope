#include "executor.h"
#include "llvm/ExecutionEngine/Orc/ThreadSafeModule.h"
#include "llvm/ExecutionEngine/Orc/ExecutionUtils.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Transforms/InstCombine/InstCombine.h"
#include "llvm/Transforms/Scalar.h"
#include "llvm/Transforms/Scalar/GVN.h"
#include <iostream>

using namespace llvm;
using namespace llvm::orc;

namespace kscope {

Executor::Executor(Box<LLJIT> lljit)
    : lljit_(std::move(lljit)), dylib_(lljit_->getMainJITDylib()) {
  dylib_.addGenerator(cantFail(
    DynamicLibrarySearchGenerator::GetForCurrentProcess(
      lljit_->getDataLayout().getGlobalPrefix())));
}

Box<Executor> Executor::create() {
  auto lljit = cantFail(LLJITBuilder().create());
  return std::make_unique<Executor>(std::move(lljit));
}

void Executor::init_native_target() {
  InitializeNativeTarget();
  InitializeNativeTargetAsmPrinter();
  InitializeNativeTargetAsmParser();
}

ResourceTrackerSP Executor::add_module(Box<Module> mod, ResourceTrackerSP tracker) {
  if (!tracker) {
    tracker = dylib_.createResourceTracker();
  }
  auto ctx = std::make_unique<LLVMContext>();
  ThreadSafeModule tsm(std::move(mod), std::move(ctx));
  cantFail(lljit_->addIRModule(tracker, std::move(tsm)));
  return tracker;
}

void Executor::remove_module(ResourceTrackerSP tracker) {
  cantFail(tracker->remove());
}

Expected<JITEvaluatedSymbol> Executor::lookup(StringRef name) {
  return lljit_->lookup(name);
}

} // namespace kscope

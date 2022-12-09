#pragma once

#include "common.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/ExecutionEngine/JITSymbol.h"
#include "llvm/ExecutionEngine/Orc/Core.h"
#include "llvm/ExecutionEngine/Orc/LLJIT.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/Error.h"

namespace kscope {

class Executor {
public:
  static void init_native_target();
  static Box<Executor> create();

  Executor(Box<llvm::orc::LLJIT> lljit);

  llvm::orc::ResourceTrackerSP add_module(Box<llvm::Module> mod,
                                          llvm::orc::ResourceTrackerSP tracker = nullptr);
  void remove_module(llvm::orc::ResourceTrackerSP tracker);
  llvm::Expected<llvm::orc::ExecutorAddr> lookup(llvm::StringRef name);

  const llvm::DataLayout& data_layout() const {
    return lljit_->getDataLayout();
  }

  llvm::orc::JITDylib& main_dylib() const {
    return dylib_;
  }

private:
  Box<llvm::orc::LLJIT> lljit_;
  llvm::orc::JITDylib& dylib_;
};

} // namespace kscope

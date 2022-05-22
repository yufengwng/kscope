#pragma once

#include "ast.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Value.h"
#include <map>

namespace kscope {

class Emitter {
public:
  Emitter(const std::string& mod_name);

  /// Generate LLVM IR for function definition.
  llvm::Function* codegen(const FunctionAST* ast);

  /// Generate LLVM IR for extern prototype.
  llvm::Function* codegen(const PrototypeAST* ast);

  /// Returns a shared pointer to the current LLVM module.
  llvm::Module* mod() const {
    return module_.get();
  }

  bool errored() const {
    return errored_;
  }

private:
  bool errored_;
  Box<llvm::LLVMContext> ctx_;
  Box<llvm::Module> module_;
  Box<llvm::IRBuilder<>> builder_;
  std::map<std::string, llvm::Value*> locals_;

  llvm::Function* emit_def(const FunctionAST* def);
  llvm::Function* emit_proto(const PrototypeAST* proto);
  llvm::Value* emit_expr(const ExprAST* expr);
  llvm::Value* emit_num_expr(const NumExprAST* num);
  llvm::Value* emit_var_expr(const VarExprAST* var);
  llvm::Value* emit_bin_expr(const BinExprAST* bin);
  llvm::Value* emit_call_expr(const CallExprAST* call);

  /// Helper for error handling.
  llvm::Value* log_err(llvm::StringRef msg);
  /// Helper for error handling typed to function values.
  llvm::Function* log_err_fn(llvm::StringRef msg);
};

} // namespace kscope

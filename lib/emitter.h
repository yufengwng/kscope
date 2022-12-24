#pragma once

#include "ast.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Value.h"
#include <map>

namespace kscope {

class Optimizer {
public:
  Optimizer(llvm::Module* mod);

  /// Optimize the given function.
  void run(llvm::Function* fn);

private:
  Box<llvm::legacy::FunctionPassManager> fpm_;
};

class Emitter {
public:
  Emitter(const std::string& mod_name, const llvm::DataLayout& layout);

  /// Returns the current module and initializes a fresh new module.
  Box<llvm::Module> take_mod();

  /// Track the given prototype in the mapping.
  void register_proto(Box<PrototypeAST> proto);

  /// Generate LLVM IR for function definition.
  llvm::Function* codegen(const FunctionAST* ast);

  /// Generate LLVM IR for extern prototype.
  llvm::Function* codegen(const PrototypeAST* ast);

  bool errored() const {
    return errored_;
  }

private:
  bool errored_;
  Box<llvm::LLVMContext> ctx_;
  Box<llvm::Module> module_;
  Box<llvm::IRBuilder<>> builder_;
  Box<Optimizer> opt_;
  std::map<std::string, llvm::Value*> locals_;
  std::map<std::string, Box<PrototypeAST>> protos_;

  llvm::Function* lookup_fn(const std::string& name);

  llvm::Function* emit_proto(const PrototypeAST* proto);
  llvm::Function* emit_def(const FunctionAST* def);
  llvm::Value* emit_expr(const ExprAST* expr);
  llvm::Value* emit_num_expr(const NumExprAST* num);
  llvm::Value* emit_var_expr(const VarExprAST* var);
  llvm::Value* emit_bin_expr(const BinExprAST* bin);
  llvm::Value* emit_call_expr(const CallExprAST* call);
  llvm::Value* emit_if_expr(const IfExprAST* ifexpr);
  llvm::Value* emit_for_expr(const ForExprAST* forexpr);

  /// Helper for error handling.
  llvm::Value* log_err(llvm::StringRef msg);
  /// Helper for error handling typed to function values.
  llvm::Function* log_err_fn(llvm::StringRef msg);
};

} // namespace kscope

#include "emitter.h"
#include "llvm/ADT/APFloat.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Verifier.h"
#include "llvm/Support/raw_ostream.h"
#include <iostream>

using namespace llvm;

namespace kscope {

Emitter::Emitter(const std::string& mod_name) {
  ctx_ = std::make_unique<LLVMContext>();
  module_ = std::make_unique<Module>(mod_name, *ctx_);
  builder_ = std::make_unique<IRBuilder<>>(*ctx_);
  errored_ = false;
}

Function* Emitter::codegen(const FunctionAST* ast) {
  return emit_def(ast);
}

Function* Emitter::codegen(const PrototypeAST* ast) {
  return emit_proto(ast);
}

Function* Emitter::emit_def(const FunctionAST* def) {
  auto* proto = def->proto();
  auto* fn = module_->getFunction(proto->name());
  if (!fn) {
    fn = emit_proto(proto);
    if (!fn) {
      return nullptr;
    }
  } else {
    // Validate existing declaration matches prototype.
    if (fn->getName() != proto->name()) {
      return log_err_fn("function name mismatch: " + proto->name());
    }
    if (fn->arg_size() != proto->num_args()) {
      return log_err_fn("function arity mismatch: " + proto->name());
    }
    for(size_t i = 0; i < fn->arg_size(); i++) {
      auto fn_arg_name = fn->getArg(i)->getName();
      auto proto_arg_name = proto->args()[i];
      if (fn_arg_name != proto_arg_name) {
        return log_err_fn("function arg unknown: " + proto_arg_name);
      }
    }
  }

  if (!fn->empty()) {
    return log_err_fn("function cannot be redefined: " + proto->name());
  }

  auto* bb = BasicBlock::Create(*ctx_, "entry", fn);
  builder_->SetInsertPoint(bb);

  locals_.clear();
  for (auto& arg : fn->args()) {
    locals_[arg.getName().str()] = &arg;
  }

  if (auto* val = emit_expr(def->body())) {
    builder_->CreateRet(val);
    std::string buf;
    raw_string_ostream stream(buf);
    if (verifyFunction(*fn, &stream)) {
      return log_err_fn("incorrect llvm function: " + stream.str());
    }
    return fn;
  }

  fn->eraseFromParent();
  return nullptr;
}

Function* Emitter::emit_proto(const PrototypeAST* proto) {
  auto* double_ty = Type::getDoubleTy(*ctx_);
  std::vector<Type*> param_tys(proto->num_args(), double_ty);
  auto* fn_ty = FunctionType::get(double_ty, param_tys, false);
  auto* fn = Function::Create(fn_ty, Function::ExternalLinkage, proto->name(), module_.get());

  size_t idx = 0;
  for (auto& arg : fn->args()) {
    arg.setName(proto->args()[idx]);
    idx++;
  }

  return fn;
}

Value* Emitter::emit_expr(const ExprAST* expr) {
  if (auto* num = dyn_cast<NumExprAST>(expr)) {
    return emit_num_expr(num);
  } else if (auto* var = dyn_cast<VarExprAST>(expr)) {
    return emit_var_expr(var);
  } else if (auto* bin = dyn_cast<BinExprAST>(expr)) {
    return emit_bin_expr(bin);
  } else if (auto* call = dyn_cast<CallExprAST>(expr)) {
    return emit_call_expr(call);
  } else {
    return nullptr;
  }
}

Value* Emitter::emit_num_expr(const NumExprAST* num) {
  return ConstantFP::get(*ctx_, APFloat(num->value()));
}

Value* Emitter::emit_var_expr(const VarExprAST* var) {
  Value* val = locals_[var->name()];
  if (!val) {
    return log_err("unknown variable name: " + var->name());
  }
  return val;
}

Value* Emitter::emit_bin_expr(const BinExprAST* bin) {
  auto* lval = emit_expr(bin->lhs());
  auto* rval = emit_expr(bin->rhs());
  if (!lval || !rval) {
    return nullptr;
  }

  switch (bin->op()) {
  case '+':
    return builder_->CreateFAdd(lval, rval);
  case '-':
    return builder_->CreateFSub(lval, rval);
  case '*':
    return builder_->CreateFMul(lval, rval);
  case '<': {
    auto* res = builder_->CreateFCmpULT(lval, rval);
    return builder_->CreateUIToFP(res, Type::getDoubleTy(*ctx_));
  }
  default:
    return log_err("invalid binary operator: " + bin->op());
  }
}

Value* Emitter::emit_call_expr(const CallExprAST* call) {
  // Lookup name in module's global symbol table.
  Function* callee = module_->getFunction(call->callee());
  if (!callee) {
    return log_err("unknown function: " + call->callee());
  }

  // Check function argument arity.
  if (callee->arg_size() != call->num_args()) {
    return log_err("incorrect number of arguments passed");
  }

  std::vector<Value*> arg_vals;
  for (auto& arg : call->args()) {
    auto* val = emit_expr(arg.get());
    if (!val) {
      return nullptr;
    }
    arg_vals.push_back(val);
  }

  return builder_->CreateCall(callee, arg_vals);
}

Value* Emitter::log_err(StringRef msg) {
  std::cerr << "[error] " << msg.str() << std::endl;
  errored_ = true;
  return nullptr;
}

Function* Emitter::log_err_fn(StringRef msg) {
  log_err(msg);
  return nullptr;
}

} // namespace kscope

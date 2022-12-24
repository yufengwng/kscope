#include "emitter.h"
#include "llvm/ADT/APFloat.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Verifier.h"
#include "llvm/Support/raw_ostream.h"
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
  // Eliminate redundant expressions.
  fpm_->add(createGVNPass());
  // Simplify control flow graph.
  fpm_->add(createCFGSimplificationPass());
  fpm_->doInitialization();
}

void Optimizer::run(Function* fn) {
  fpm_->run(*fn);
}

Emitter::Emitter(const std::string& mod_name, const DataLayout& layout) {
  ctx_ = std::make_unique<LLVMContext>();
  builder_ = std::make_unique<IRBuilder<>>(*ctx_);
  module_ = std::make_unique<Module>(mod_name, *ctx_);
  module_->setDataLayout(layout);
  opt_ = std::make_unique<Optimizer>(module_.get());
  errored_ = false;
}

Box<llvm::Module> Emitter::take_mod() {
  auto curr_mod = std::move(module_);
  module_ = std::make_unique<Module>(curr_mod->getName(), *ctx_);
  module_->setDataLayout(curr_mod->getDataLayout());
  opt_ = std::make_unique<Optimizer>(module_.get());
  errored_ = false;
  return curr_mod;
}

void Emitter::register_proto(Box<PrototypeAST> proto) {
  protos_[proto->name()] = std::move(proto);
}

Function* Emitter::codegen(const FunctionAST* ast) {
  errored_ = false;
  return emit_def(ast);
}

Function* Emitter::codegen(const PrototypeAST* ast) {
  errored_ = false;
  return emit_proto(ast);
}

Function* Emitter::lookup_fn(const std::string& name) {
  if (auto* fn = module_->getFunction(name)) {
    return fn;
  }
  auto iter = protos_.find(name);
  if (iter != protos_.end()) {
    return emit_proto(iter->second.get());
  }
  return nullptr;
}

Function* Emitter::emit_proto(const PrototypeAST* proto) {
  auto* double_ty = Type::getDoubleTy(*ctx_);
  std::vector<Type*> param_tys(proto->num_args(), double_ty);
  auto* fn_ty = FunctionType::get(double_ty, param_tys, false);
  auto* fn = Function::Create(fn_ty, Function::ExternalLinkage,
                              proto->name(), module_.get());

  size_t idx = 0;
  for (auto& arg : fn->args()) {
    arg.setName(proto->args()[idx]);
    idx++;
  }

  return fn;
}

Function* Emitter::emit_def(const FunctionAST* def) {
  auto* proto = def->proto();
  protos_[proto->name()] = def->clone_proto();
  auto* fn = lookup_fn(proto->name());
  if (!fn) {
    return nullptr;
  }

  if (fn->empty()) {
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

  auto* bb = BasicBlock::Create(*ctx_, "entry", fn);
  builder_->SetInsertPoint(bb);

  locals_.clear();
  for (auto& arg : fn->args()) {
    locals_[arg.getName().str()] = &arg;
  }

  if (auto* val = emit_expr(def->body())) {
    // Finish the function.
    builder_->CreateRet(val);

    // Validate generated IR.
    std::string buf;
    raw_string_ostream stream(buf);
    if (verifyFunction(*fn, &stream)) {
      return log_err_fn("incorrect llvm function: " + stream.str());
    }

    // Optimize the code.
    opt_->run(fn);

    return fn;
  }

  // There was an error so remove the function.
  fn->eraseFromParent();
  return nullptr;
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
  } else if (auto* ifexpr = dyn_cast<IfExprAST>(expr)) {
    return emit_if_expr(ifexpr);
  } else if (auto* forexpr = dyn_cast<ForExprAST>(expr)) {
    return emit_for_expr(forexpr);
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
    return log_err("invalid binary operator: " + std::string(1, bin->op()));
  }
}

Value* Emitter::emit_call_expr(const CallExprAST* call) {
  // Lookup name in module's global symbol table.
  Function* callee = lookup_fn(call->callee());
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

Value* Emitter::emit_if_expr(const IfExprAST* ifexpr) {
  // Create blocks for 'then' and 'else' cases.
  auto* fn = builder_->GetInsertBlock()->getParent();
  auto* bb_then = BasicBlock::Create(*ctx_, "then");
  auto* bb_else = BasicBlock::Create(*ctx_, "else");
  auto* bb_merge = BasicBlock::Create(*ctx_, "ifend");

  // Emit the if condition.
  auto* cond_val = emit_expr(ifexpr->cond_expr());
  if (!cond_val) {
    return nullptr;
  }
  cond_val = builder_->CreateFCmpONE(
      cond_val, ConstantFP::get(*ctx_, APFloat(0.0)));
  builder_->CreateCondBr(cond_val, bb_then, bb_else);

  // Emit the 'then' branch.
  fn->getBasicBlockList().push_back(bb_then);
  builder_->SetInsertPoint(bb_then);
  auto* then_val = emit_expr(ifexpr->then_expr());
  if (!then_val) {
    return nullptr;
  }
  builder_->CreateBr(bb_merge);
  bb_then = builder_->GetInsertBlock();

  // Emit the 'else' branch.
  fn->getBasicBlockList().push_back(bb_else);
  builder_->SetInsertPoint(bb_else);
  auto* else_val = emit_expr(ifexpr->else_expr());
  if (!else_val) {
    return nullptr;
  }
  builder_->CreateBr(bb_merge);
  bb_else = builder_->GetInsertBlock();

  // Emit the merge block.
  fn->getBasicBlockList().push_back(bb_merge);
  builder_->SetInsertPoint(bb_merge);
  auto* phi = builder_->CreatePHI(Type::getDoubleTy(*ctx_), 2, "ifphi");
  phi->addIncoming(then_val, bb_then);
  phi->addIncoming(else_val, bb_else);

  return phi;
}

Value* Emitter::emit_for_expr(const ForExprAST* forexpr) {
  auto& var_name = forexpr->itervar();
  auto* zero_val = ConstantFP::get(*ctx_, APFloat(0.0));

  // Create blocks for the loop.
  auto* fn = builder_->GetInsertBlock()->getParent();
  auto* bb_preheader = BasicBlock::Create(*ctx_, "loop.pre");
  auto* bb_loop_cond = BasicBlock::Create(*ctx_, "loop.cond");
  auto* bb_cond_less = BasicBlock::Create(*ctx_, "loop.cond.less");
  auto* bb_cond_else = BasicBlock::Create(*ctx_, "loop.cond.else");
  auto* bb_loop_body = BasicBlock::Create(*ctx_, "loop.body");
  auto* bb_loop_post = BasicBlock::Create(*ctx_, "loop.post");
  auto* bb_loop_end = BasicBlock::Create(*ctx_, "loop.end");

  // Start the loop preheader.
  fn->getBasicBlockList().push_back(bb_preheader);
  builder_->CreateBr(bb_preheader);
  builder_->SetInsertPoint(bb_preheader);

  // Emit the range bounds (these are evaluated once).
  auto* init_val = emit_expr(forexpr->init_expr());
  if (!init_val) {
    return nullptr;
  }
  auto* stop_val = emit_expr(forexpr->stop_expr());
  if (!stop_val) {
    return nullptr;
  }
  Value* step_val;
  if (forexpr->has_step()) {
    step_val = emit_expr(forexpr->step_expr());
    if (!step_val) {
      return nullptr;
    }
  } else {
    step_val = ConstantFP::get(*ctx_, APFloat(ForExprAST::DEFAULT_STEP));
  }
  bb_preheader = builder_->GetInsertBlock();

  // Start loop condition block.
  fn->getBasicBlockList().push_back(bb_loop_cond);
  builder_->CreateBr(bb_loop_cond);
  builder_->SetInsertPoint(bb_loop_cond);

  // Emit the phi node for the itervar.
  auto* iter_phi = builder_->CreatePHI(Type::getDoubleTy(*ctx_), 2, var_name);
  iter_phi->addIncoming(init_val, bb_preheader);

  // Save variable in case it is shadowed by the itervar.
  auto* old_val = locals_[var_name];
  locals_[var_name] = iter_phi;

  // Check range condition based on step direction.
  auto* cmp = builder_->CreateFCmpULT(step_val, zero_val);
  builder_->CreateCondBr(cmp, bb_cond_less, bb_cond_else);

  // if step < 0 then execute loop if iter > stop
  fn->getBasicBlockList().push_back(bb_cond_less);
  builder_->SetInsertPoint(bb_cond_less);
  cmp = builder_->CreateFCmpUGT(iter_phi, stop_val);
  builder_->CreateCondBr(cmp, bb_loop_body, bb_loop_end);

  // if step >= 0 then execute loop if iter < stop
  fn->getBasicBlockList().push_back(bb_cond_else);
  builder_->SetInsertPoint(bb_cond_else);
  cmp = builder_->CreateFCmpULT(iter_phi, stop_val);
  builder_->CreateCondBr(cmp, bb_loop_body, bb_loop_end);

  // Emit the loop body. Its value is ignored.
  fn->getBasicBlockList().push_back(bb_loop_body);
  builder_->SetInsertPoint(bb_loop_body);
  if (!emit_expr(forexpr->body_expr())) {
    return nullptr;
  }
  builder_->CreateBr(bb_loop_post);

  // Emit the step and add backedge.
  fn->getBasicBlockList().push_back(bb_loop_post);
  builder_->SetInsertPoint(bb_loop_post);
  auto* next_val = builder_->CreateFAdd(iter_phi, step_val, "next");
  iter_phi->addIncoming(next_val, bb_loop_post);
  builder_->CreateBr(bb_loop_cond);

  // Rest of codegen goes in the loop end.
  fn->getBasicBlockList().push_back(bb_loop_end);
  builder_->SetInsertPoint(bb_loop_end);

  // Restore the shadowed variable, if any.
  if (old_val) {
    locals_[var_name] = old_val;
  } else {
    locals_.erase(var_name);
  }

  // For now, for/in expression always returns zero.
  return zero_val;
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

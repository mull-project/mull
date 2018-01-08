#include "Instrumentation/Callbacks.h"
#include "Instrumentation/DynamicCallTree.h"
#include "Instrumentation/Instrumentation.h"
#include "Driver.h"

#include <llvm/ExecutionEngine/Orc/JITSymbol.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/GlobalVariable.h>

using namespace mull;
using namespace llvm;

namespace mull {

extern "C" void mull_enterFunction(InstrumentationInfo *info, uint64_t functionIndex) {
  assert(info);
  assert(info->callTreeMapping());
  DynamicCallTree::enterFunction(functionIndex,
                                 info->callTreeMapping(),
                                 info->callstack());
}

extern "C" void mull_leaveFunction(InstrumentationInfo *info, uint64_t functionIndex) {
  assert(info);
  assert(info->callTreeMapping());
  DynamicCallTree::leaveFunction(functionIndex,
                                 info->callTreeMapping(),
                                 info->callstack());
}

}

Callbacks::Callbacks() {}

Value *Callbacks::injectInstrumentationInfoPointer(llvm::Module *module) {
  auto &context = module->getContext();
  auto driverPointerType = Type::getVoidTy(context)->getPointerTo();
  return module->getOrInsertGlobal("mull_instrumentation_info", driverPointerType);
}

void Callbacks::injectCallbacks(llvm::Function *function, uint64_t index, Value *infoPointer) {
  auto &context = function->getParent()->getContext();
  auto int64Type = Type::getInt64Ty(context);
  auto driverPointerType = Type::getVoidTy(context)->getPointerTo();
  auto voidType = Type::getVoidTy(context);
  std::vector<Type *> parameterTypes({driverPointerType, int64Type});

  FunctionType *callbackType = FunctionType::get(voidType, parameterTypes, false);

  Value *functionIndex = ConstantInt::get(int64Type, index);
  std::vector<Value *> parameters({infoPointer, functionIndex});

  Function *enterFunction = function->getParent()->getFunction("mull_enterFunction");
  Function *leaveFunction = function->getParent()->getFunction("mull_leaveFunction");

  if (enterFunction == nullptr && leaveFunction == nullptr) {
    enterFunction = Function::Create(callbackType,
                                     Function::ExternalLinkage,
                                     "mull_enterFunction",
                                     function->getParent());

    leaveFunction = Function::Create(callbackType,
                                     Function::ExternalLinkage,
                                     "mull_leaveFunction",
                                     function->getParent());
  }

  assert(enterFunction);
  assert(leaveFunction);

  auto &entryBlock = *function->getBasicBlockList().begin();
  CallInst *enterFunctionCall = CallInst::Create(enterFunction, parameters);
  enterFunctionCall->insertBefore(&*entryBlock.getInstList().begin());

  for (auto &block : function->getBasicBlockList()) {
    ReturnInst *returnStatement = nullptr;
    if (!(returnStatement = dyn_cast<ReturnInst>(block.getTerminator()))) {
      continue;
    }

    CallInst *leaveFunctionCall = CallInst::Create(leaveFunction, parameters);
    leaveFunctionCall->insertBefore(returnStatement);
  }
}

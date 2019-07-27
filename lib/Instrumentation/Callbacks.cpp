#include "mull/Instrumentation/Callbacks.h"

#include "mull/Bitcode.h"
#include "mull/Driver.h"
#include "mull/Instrumentation/DynamicCallTree.h"
#include "mull/Instrumentation/Instrumentation.h"

#include <llvm/IR/Constants.h>
#include <llvm/IR/GlobalVariable.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/Module.h>

using namespace mull;
using namespace llvm;

namespace mull {

extern "C" void mull_enterFunction(void **trampoline, uint32_t functionIndex) {
  InstrumentationInfo *info = (InstrumentationInfo *)*trampoline;
  assert(info);
  assert(info->callTreeMapping);
  DynamicCallTree::enterFunction(functionIndex, info->callTreeMapping,
                                 info->callstack);
}

extern "C" void mull_leaveFunction(void **trampoline, uint32_t functionIndex) {
  InstrumentationInfo *info = (InstrumentationInfo *)*trampoline;
  assert(info);
  assert(info->callTreeMapping);
  DynamicCallTree::leaveFunction(functionIndex, info->callTreeMapping,
                                 info->callstack);
}

} // namespace mull

Value *Callbacks::injectInstrumentationInfoPointer(Module *module,
                                                   const char *variableName) {
  auto &context = module->getContext();
  auto trampolineType = Type::getInt8Ty(context)->getPointerTo();
  return module->getOrInsertGlobal(variableName, trampolineType);
}

Value *
Callbacks::injectFunctionIndexOffset(Module *module,
                                     const char *functionIndexOffsetPrefix) {
  auto &context = module->getContext();
  auto functionIndexOffsetType = Type::getInt32Ty(context);
  std::string functionIndexOffset(functionIndexOffsetPrefix);
  functionIndexOffset += module->getModuleIdentifier();

  return module->getOrInsertGlobal(functionIndexOffset,
                                   functionIndexOffsetType);
}

void Callbacks::injectCallbacks(llvm::Function *function, uint32_t index,
                                Value *infoPointer, Value *offset) {
  auto &context = function->getParent()->getContext();
  auto intType = Type::getInt32Ty(context);
  auto trampolineType =
      Type::getInt8Ty(context)->getPointerTo()->getPointerTo();
  auto voidType = Type::getVoidTy(context);
  std::vector<Type *> parameterTypes({trampolineType, intType});

  FunctionType *callbackType =
      FunctionType::get(voidType, parameterTypes, false);

  Function *enterFunction =
      function->getParent()->getFunction("mull_enterFunction");
  Function *leaveFunction =
      function->getParent()->getFunction("mull_leaveFunction");

  if (enterFunction == nullptr && leaveFunction == nullptr) {
    enterFunction =
        Function::Create(callbackType, Function::ExternalLinkage,
                         "mull_enterFunction", function->getParent());

    leaveFunction =
        Function::Create(callbackType, Function::ExternalLinkage,
                         "mull_leaveFunction", function->getParent());
  }

  assert(enterFunction);
  assert(leaveFunction);

  Value *functionIndex = ConstantInt::get(intType, index);

  auto &entryBlock = *function->getBasicBlockList().begin();
  auto firstInstruction = &*entryBlock.getInstList().begin();

  Value *offsetValue = new LoadInst(offset, "offset", firstInstruction);
  Value *indexAndOffset =
      BinaryOperator::Create(Instruction::Add, functionIndex, offsetValue,
                             "functionIndex", firstInstruction);
  std::vector<Value *> enterParameters({infoPointer, indexAndOffset});

  CallInst *enterFunctionCall =
      CallInst::Create(enterFunction, enterParameters);
  enterFunctionCall->insertBefore(firstInstruction);

  for (auto &block : function->getBasicBlockList()) {
    ReturnInst *returnStatement = nullptr;
    if (!(returnStatement = dyn_cast<ReturnInst>(block.getTerminator()))) {
      continue;
    }

    Value *offsetValue = new LoadInst(offset, "offset", returnStatement);
    Value *indexAndOffset =
        BinaryOperator::Create(Instruction::Add, functionIndex, offsetValue,
                               "functionIndex", returnStatement);
    std::vector<Value *> leaveParameters({infoPointer, indexAndOffset});

    CallInst *leaveFunctionCall =
        CallInst::Create(leaveFunction, leaveParameters);
    leaveFunctionCall->insertBefore(returnStatement);
  }
}

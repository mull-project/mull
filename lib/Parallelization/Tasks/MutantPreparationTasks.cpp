#include "mull/Parallelization/Tasks/MutantPreparationTasks.h"
#include "mull/MutationPoint.h"
#include "mull/Parallelization/Progress.h"
#include <llvm/IR/Constant.h>
#include <llvm/IR/Instructions.h>
#include <llvm/Transforms/Utils/Cloning.h>

using namespace mull;

void CloneMutatedFunctionsTask::operator()(iterator begin, iterator end, Out &storage,
                                           progress_counter &counter) {
  for (auto it = begin; it != end; it++, counter.increment()) {
    Bitcode &bitcode = **it;
    cloneFunctions(bitcode, storage);
  }
}

void CloneMutatedFunctionsTask::cloneFunctions(Bitcode &bitcode, Out &mutatedFunctions) {
  for (auto pair : bitcode.getMutationPointsMap()) {
    llvm::Function *original = pair.first;
    MutationPoint *anyPoint = pair.second.front();
    mutatedFunctions.push_back(anyPoint->getTrampolineName());

    for (MutationPoint *point : pair.second) {
      llvm::ValueToValueMapTy map;
      llvm::Function *mutatedFunction = llvm::CloneFunction(original, map);
      point->setMutatedFunction(mutatedFunction);
    }
  }
}

void DeleteOriginalFunctionsTask::operator()(iterator begin, iterator end, Out &storage,
                                             progress_counter &counter) {
  for (auto it = begin; it != end; it++, counter.increment()) {
    Bitcode &bitcode = **it;
    deleteFunctions(bitcode);
  }
}

void DeleteOriginalFunctionsTask::deleteFunctions(Bitcode &bitcode) {
  for (auto pair : bitcode.getMutationPointsMap()) {
    auto original = pair.first;
    auto anyPoint = pair.second.front();

    llvm::ValueToValueMapTy map;
    auto originalCopy = CloneFunction(original, map);
    originalCopy->setName(anyPoint->getOriginalFunctionName());
    original->deleteBody();
  }
}

void InsertMutationTrampolinesTask::operator()(iterator begin, iterator end, Out &storage,
                                               progress_counter &counter) {
  for (auto it = begin; it != end; it++, counter.increment()) {
    Bitcode &bitcode = **it;
    insertTrampolines(bitcode);
  }
}

void InsertMutationTrampolinesTask::insertTrampolines(Bitcode &bitcode) {
  llvm::Module *module = bitcode.getModule();
  llvm::LLVMContext &context = module->getContext();
  for (auto pair : bitcode.getMutationPointsMap()) {
    auto original = pair.first;
    auto anyPoint = pair.second.front();
    std::vector<llvm::Value *> args;

    for (auto &arg : original->args()) {
      args.push_back(&arg);
    }

    llvm::Type *functionPointer = original->getFunctionType()->getPointerTo();
    auto trampoline = module->getOrInsertGlobal(anyPoint->getTrampolineName(), functionPointer);
    llvm::BasicBlock *block = llvm::BasicBlock::Create(context, "indirect_call", original);

    auto retType = original->getFunctionType()->getReturnType();
    llvm::Constant *dummy = nullptr;
    if (!retType->isVoidTy()) {
      dummy = llvm::Constant::getNullValue(retType);
    }
    auto retVal = llvm::ReturnInst::Create(context, dummy, block);

    auto loadValue = new llvm::LoadInst(trampoline->getType()->getPointerElementType(),
                                        trampoline,
                                        "indirect_function_pointer",
                                        false,
                                        retVal);
    auto callInst =
        llvm::CallInst::Create(original->getFunctionType(), loadValue, args, "", retVal);
    if (!retType->isVoidTy()) {
      retVal->setOperand(0, callInst);
    }
  }
}

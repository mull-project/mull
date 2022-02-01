#include "mull/Parallelization/Tasks/MutantPreparationTasks.h"
#include "mull/Config/Configuration.h"
#include "mull/MutationPoint.h"
#include "mull/Parallelization/Progress.h"
#include <llvm/IR/Constant.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/Instructions.h>
#include <llvm/Transforms/Utils/Cloning.h>

using namespace mull;

void CloneMutatedFunctionsTask::operator()(iterator begin, iterator end, Out &storage,
                                           progress_counter &counter) {
  for (auto it = begin; it != end; it++, counter.increment()) {
    Bitcode &bitcode = **it;
    cloneFunctions(bitcode);
  }
}

void CloneMutatedFunctionsTask::cloneFunctions(Bitcode &bitcode) {
  for (auto &pair : bitcode.getMutationPointsMap()) {
    llvm::Function *original = pair.first;
    for (MutationPoint *point : pair.second) {
      llvm::ValueToValueMapTy map;
      llvm::Function *mutatedFunction = llvm::CloneFunction(original, map);
      mutatedFunction->setLinkage(llvm::GlobalValue::InternalLinkage);
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
  for (auto &pair : bitcode.getMutationPointsMap()) {
    auto original = pair.first;
    /// Remove the original function if at least one mutant is covered
    for (MutationPoint *point : pair.second) {
      llvm::ValueToValueMapTy map;
      auto originalCopy = CloneFunction(original, map);
      originalCopy->setName(point->getOriginalFunctionName());
      original->dropAllReferences();
      break;
    }
  }
}

void InsertMutationTrampolinesTask::operator()(iterator begin, iterator end, Out &storage,
                                               progress_counter &counter) {
  for (auto it = begin; it != end; it++, counter.increment()) {
    Bitcode &bitcode = **it;
    insertTrampolines(bitcode, configuration);
  }
}
// declare i32 @printf(i8*, ...)
// call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @.str, i64 0, i64 0))
static void insertTrace(llvm::BasicBlock *basicBlock, const std::string &format,
                        const std::string &message) {
  auto module = basicBlock->getParent()->getParent();
  auto &context = module->getContext();
  llvm::Type *intType = llvm::Type::getInt32Ty(context);
  llvm::Type *charPtr = llvm::Type::getInt8Ty(context)->getPointerTo();
  llvm::FunctionType *printfType = llvm::FunctionType::get(intType, { charPtr }, true);
  auto print = module->getOrInsertFunction("printf", printfType).getCallee();

  auto formatArray = llvm::ConstantDataArray::getString(context, format);
  auto messageArray = llvm::ConstantDataArray::getString(context, message);
  auto *fmt = new llvm::GlobalVariable(
      *module, formatArray->getType(), true, llvm::GlobalValue::PrivateLinkage, formatArray);
  auto *msg = new llvm::GlobalVariable(
      *module, messageArray->getType(), true, llvm::GlobalValue::PrivateLinkage, messageArray);

  llvm::Value *zero = llvm::Constant::getNullValue(llvm::Type::getInt64Ty(context));
  auto fmtGEP =
      llvm::ConstantExpr::getInBoundsGetElementPtr(formatArray->getType(), fmt, { zero, zero });
  auto msgGEP =
      llvm::ConstantExpr::getInBoundsGetElementPtr(messageArray->getType(), msg, { zero, zero });
  llvm::CallInst::Create(printfType, print, { fmtGEP, msgGEP }, "trace", basicBlock);
}

void InsertMutationTrampolinesTask::insertTrampolines(Bitcode &bitcode,
                                                      const Configuration &configuration) {
  llvm::Module *module = bitcode.getModule();
  llvm::LLVMContext &context = module->getContext();
  llvm::Type *charPtr = llvm::Type::getInt8Ty(context)->getPointerTo();
  llvm::FunctionType *getEnvType = llvm::FunctionType::get(charPtr, { charPtr }, false);
  llvm::Value *getenv = module->getOrInsertFunction("getenv", getEnvType).getCallee();
  for (auto pair : bitcode.getMutationPointsMap()) {
    llvm::Function *original = pair.first;

    llvm::BasicBlock *entry = llvm::BasicBlock::Create(context, "entry", original);
    llvm::BasicBlock *originalBlock = llvm::BasicBlock::Create(context, "original", original);
    if (configuration.debug.traceMutants) {
      insertTrace(
          originalBlock, "mull-trace: jumping over to original %s\n", original->getName().str());
    }
    llvm::BasicBlock *trampolineCall =
        llvm::BasicBlock::Create(context, "trampoline_call", original);

    if (configuration.debug.traceMutants) {
      insertTrace(trampolineCall, "mull-trace: trampoline call %s\n", original->getName().str());
    }
    auto anyPoint = pair.second.front();
    llvm::Type *trampolineType = original->getFunctionType()->getPointerTo();
    auto trampoline = new llvm::AllocaInst(trampolineType, 0, "trampoline", entry);
    if (configuration.debug.traceMutants) {
      insertTrace(entry, "mull-trace: entering %s\n", original->getName().str());
    }
    new llvm::StoreInst(bitcode.getModule()->getFunction(anyPoint->getOriginalFunctionName()),
                        trampoline,
                        originalBlock);
    llvm::BranchInst::Create(trampolineCall, originalBlock);
    llvm::BasicBlock *head = originalBlock;

    for (auto &point : pair.second) {
      auto name = llvm::ConstantDataArray::getString(context, point->getUserIdentifier());
      auto *global = new llvm::GlobalVariable(
          *module, name->getType(), true, llvm::GlobalValue::PrivateLinkage, name);

      llvm::BasicBlock *mutationCheckBlock =
          llvm::BasicBlock::Create(context, point->getUserIdentifier() + "_check", original);
      if (configuration.debug.traceMutants) {
        insertTrace(
            mutationCheckBlock, "mull-trace: checking for %s\n", point->getUserIdentifier());
      }
      llvm::Value *nullPtr = llvm::Constant::getNullValue(charPtr);
      llvm::CmpInst *predicate = llvm::CmpInst::Create(llvm::Instruction::ICmp,
                                                       llvm::ICmpInst::ICMP_NE,
                                                       nullPtr,
                                                       nullPtr,
                                                       "is_enabled",
                                                       mutationCheckBlock);
      llvm::Value *zero = llvm::Constant::getNullValue(llvm::Type::getInt64Ty(context));
      auto mutantName =
          llvm::ConstantExpr::getInBoundsGetElementPtr(name->getType(), global, { zero, zero });
      auto getEnvCall =
          llvm::CallInst::Create(getEnvType, getenv, { mutantName }, "check_mutation", predicate);
      predicate->setOperand(0, getEnvCall);

      llvm::BasicBlock *mutationBlock =
          llvm::BasicBlock::Create(context, point->getUserIdentifier(), original);
      if (configuration.debug.traceMutants) {
        insertTrace(mutationBlock, "mull-trace: jumping over to %s\n", point->getUserIdentifier());
      }
      new llvm::StoreInst(point->getMutatedFunction(), trampoline, mutationBlock);

      llvm::BranchInst::Create(mutationBlock, head, predicate, mutationCheckBlock);
      llvm::BranchInst::Create(trampolineCall, mutationBlock);
      head = mutationCheckBlock;
    }

    llvm::BranchInst::Create(head, entry);
    std::vector<llvm::Value *> args;
    for (auto &arg : original->args()) {
      args.push_back(&arg);
    }
    auto retType = original->getFunctionType()->getReturnType();
    llvm::Constant *dummy = nullptr;
    if (!retType->isVoidTy()) {
      dummy = llvm::Constant::getNullValue(retType);
    }
    auto retVal = llvm::ReturnInst::Create(context, dummy, trampolineCall);
    auto loadValue = new llvm::LoadInst(trampoline->getType()->getPointerElementType(),
                                        trampoline,
                                        "trampoline_pointer",
                                        false,
                                        retVal);
    auto callInst =
        llvm::CallInst::Create(original->getFunctionType(), loadValue, args, "", retVal);
    callInst->setAttributes(original->getAttributes());
    callInst->setCallingConv(original->getCallingConv());
    if (!retType->isVoidTy()) {
      retVal->setOperand(0, callInst);
    }
  }
}

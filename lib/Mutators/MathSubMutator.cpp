#include "mull/Mutators/MathSubMutator.h"

#include "mull/Logger.h"
#include "mull/MutationPoint.h"

#include <llvm/IR/DebugInfoMetadata.h>
#include <llvm/IR/DebugLoc.h>
#include <llvm/IR/InstIterator.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/Module.h>

#include <fstream>
#include <iterator>

using namespace llvm;
using namespace mull;

const std::string MathSubMutator::ID = "math_sub_mutator";
const std::string MathSubMutator::description = "Replaces - with +";

bool MathSubMutator::isSubWithOverflow(llvm::Value &V) {
  if (CallInst *callInst = dyn_cast<CallInst>(&V)) {
    Function *calledFunction = callInst->getCalledFunction();

    if (calledFunction == nullptr) {
      return false;
    }

    if (calledFunction->getName().startswith("llvm.ssub") ||
        calledFunction->getName().startswith("llvm.usub")) {
      return true;
    }
  }

  return false;
}

llvm::Function *
MathSubMutator::replacementForSubWithOverflow(llvm::Function *testeeFunction,
                                              llvm::Module &module) {

  std::string name = testeeFunction->getName().str();

  std::string replacementName;
  Type *replacementType = nullptr;

  if (name == "llvm.ssub.with.overflow.i8") {
    replacementName = "llvm.sadd.with.overflow.i8";
    replacementType = Type::getInt8Ty(module.getContext());
  }

  else if (name == "llvm.ssub.with.overflow.i16") {
    replacementName = "llvm.sadd.with.overflow.i16";
    replacementType = Type::getInt16Ty(module.getContext());
  }

  else if (name == "llvm.ssub.with.overflow.i32") {
    replacementName = "llvm.sadd.with.overflow.i32";
    replacementType = Type::getInt32Ty(module.getContext());
  }

  else if (name == "llvm.ssub.with.overflow.i64") {
    replacementName = "llvm.sadd.with.overflow.i64";
    replacementType = Type::getInt64Ty(module.getContext());
  }

  else if (name == "llvm.usub.with.overflow.i8") {
    replacementName = "llvm.uadd.with.overflow.i8";
    replacementType = Type::getInt8Ty(module.getContext());
  }

  else if (name == "llvm.usub.with.overflow.i16") {
    replacementName = "llvm.uadd.with.overflow.i16";
    replacementType = Type::getInt16Ty(module.getContext());
  }

  else if (name == "llvm.usub.with.overflow.i32") {
    replacementName = "llvm.uadd.with.overflow.i32";
    replacementType = Type::getInt32Ty(module.getContext());
  }

  else if (name == "llvm.usub.with.overflow.i64") {
    replacementName = "llvm.uadd.with.overflow.i64";
    replacementType = Type::getInt64Ty(module.getContext());
  }

  else {
    Logger::debug() << "MathSubMutator> unknown add function: " << name
                    << ".\n";
  }

  std::vector<Type *> twoParameters(2, replacementType);

  FunctionType *replacementFunctionType =
      FunctionType::get(replacementType, twoParameters, false);

  Function *replacementFunction =
      Function::Create(replacementFunctionType, Function::ExternalLinkage,
                       replacementName, &module);

  return replacementFunction;
}

bool MathSubMutator::canBeApplied(Value &V) {
  if (BinaryOperator *BinOp = dyn_cast<BinaryOperator>(&V)) {
    BinaryOperator::BinaryOps Opcode = BinOp->getOpcode();

    if (Opcode == Instruction::Sub || Opcode == Instruction::FSub) {
      return true;
    }
  }

  if (isSubWithOverflow(V)) {
    return true;
  }

  return false;
}

void MathSubMutator::applyMutation(Function *function,
                                   const MutationPointAddress &address) {
  llvm::Instruction &I = address.findInstruction(function);

  if (isSubWithOverflow(I)) {
    CallInst *callInst = dyn_cast<CallInst>(&I);

    Function *replacementFunction = replacementForSubWithOverflow(
        callInst->getCalledFunction(), *I.getModule());

    callInst->setCalledFunction(replacementFunction);

    return;
  }

  BinaryOperator *binaryOperator = cast<BinaryOperator>(&I);
  assert(binaryOperator->getOpcode() == Instruction::Sub ||
         binaryOperator->getOpcode() == Instruction::FSub);

  auto type = Instruction::Add;
  if (binaryOperator->getOpcode() == Instruction::FSub) {
    type = Instruction::FAdd;
  }

  /// NOTE: Create a new BinaryOperator with the same name as existing one
  Instruction *replacement = BinaryOperator::Create(
      type, binaryOperator->getOperand(0), binaryOperator->getOperand(1),
      binaryOperator->getName());
  assert(replacement);

  /// FSub does not support NUW/NSW
  if (binaryOperator->getOpcode() != Instruction::FSub) {
    if (binaryOperator->hasNoUnsignedWrap()) {
      replacement->setHasNoUnsignedWrap();
    }

    if (binaryOperator->hasNoSignedWrap()) {
      replacement->setHasNoSignedWrap();
    }
  }

  /// NOTE: If I add a named instruction, and the name already exist
  /// in a basic block, then LLVM will make another unique name of it
  /// To prevent this name change we need to 'drop' the existing old name
  /// TODO: Check behaviour of 'unnamed' instructions (e.g.: %0, %2, etc.)
  binaryOperator->setName("");

  replacement->insertAfter(binaryOperator);
  binaryOperator->replaceAllUsesWith(replacement);
  binaryOperator->eraseFromParent();
}

std::vector<MutationPoint *>
MathSubMutator::getMutations(Bitcode *bitcode, llvm::Function *function) {
  assert(bitcode);
  assert(function);

  std::vector<MutationPoint *> mutations;

  for (auto &instruction : instructions(function)) {
    if (!canBeApplied(instruction)) {
      continue;
    }

    std::string diagnostics = "Math Sub: replaced - with +";
    auto point =
        new MutationPoint(this, &instruction, diagnostics, "+", bitcode);
    mutations.push_back(point);
  }

  return mutations;
}

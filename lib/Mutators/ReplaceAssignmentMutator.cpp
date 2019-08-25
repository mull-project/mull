#include "mull/Mutators/ReplaceAssignmentMutator.h"

#include "mull/Logger.h"
#include "mull/MutationPoint.h"

#include <llvm/IR/CallSite.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/InstIterator.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/Module.h>

#include <fstream>
#include <iterator>
#include <sstream>

using namespace llvm;
using namespace mull;

const std::string ReplaceAssignmentMutator::ID = "replace_assignment_mutator";
const std::string ReplaceAssignmentMutator::description =
    "Replaces assignment with 42";

static bool findPossibleApplication(Value &V, std::string &outDiagnostics);
static llvm::Value *getReplacement(Type *returnType,
                                   llvm::LLVMContext &context);

static bool findPossibleApplication(Value &V, std::string &outDiagnostics) {

  std::string diagnostics;

  Instruction *instruction = dyn_cast<Instruction>(&V);
  assert(instruction);

  StoreInst *storeInstruction = dyn_cast<StoreInst>(&V);
  if (storeInstruction == nullptr) {
    return false;
  }

  Value *storedValue = storeInstruction->getOperand(0);
  if (storedValue->getType()->isIntegerTy() == false &&
      storedValue->getType()->isFloatTy() == false &&
      storedValue->getType()->isDoubleTy() == false) {
    return false;
  }

  std::stringstream diagnosticsStream;

  diagnosticsStream << "Replace Assignment: replaced rvalue with 42";

  outDiagnostics.assign(diagnosticsStream.str());

  return true;
}

static llvm::Value *getReplacement(Type *returnType,
                                   llvm::LLVMContext &context) {
  static const int MagicValue = 42;

  if (returnType->isIntegerTy()) {
    APInt replacementIntValue =
        APInt(returnType->getIntegerBitWidth(), MagicValue);

    return ConstantInt::get(context, replacementIntValue);
  }
  if (returnType->isDoubleTy()) {
    APFloat replacementFloatValue = APFloat((double)MagicValue);
    return ConstantFP::get(context, replacementFloatValue);
  }
  if (returnType->isFloatTy()) {
    APFloat replacementFloatValue = APFloat((float)MagicValue);
    return ConstantFP::get(context, replacementFloatValue);
  }

  llvm_unreachable("Unsupported return type!");
}

void ReplaceAssignmentMutator::applyMutation(
    Function *function, const MutationPointAddress &address) {
  llvm::Instruction &instruction = address.findInstruction(function);

  StoreInst *storeInstruction = dyn_cast<StoreInst>(&instruction);

  Value *storeOperand = storeInstruction->getOperand(0);
  auto returnedType = storeOperand->getType();

  Value *replacement = getReplacement(returnedType, instruction.getContext());

  storeInstruction->setOperand(0, replacement);
}

std::vector<MutationPoint *>
ReplaceAssignmentMutator::getMutations(Bitcode *bitcode,
                                       llvm::Function *function) {
  assert(bitcode);
  assert(function);

  std::vector<MutationPoint *> mutations;

  for (auto &instruction : instructions(function)) {
    std::string diagnostics;

    if (!findPossibleApplication(instruction, diagnostics)) {
      continue;
    }

    const std::string replacement = "42";

    auto point = new MutationPoint(this, &instruction, diagnostics, replacement,
                                   bitcode);
    mutations.push_back(point);
  }

  return mutations;
}

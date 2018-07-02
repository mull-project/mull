#include "Mutators/ReplaceAssignmentMutator.h"

#include "Context.h"
#include "Logger.h"
#include "MutationPoint.h"

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

static bool findPossibleApplication(Value &V, std::string &outDiagnostics);
static
llvm::Value *getReplacement(Type *returnType, llvm::LLVMContext &context);

bool ReplaceAssignmentMutator::canBeApplied(Value &V) {
  std::string diagnostics;

  return findPossibleApplication(V, diagnostics);
}

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

MutationPoint *
ReplaceAssignmentMutator::getMutationPoint(MullModule *module,
                                           MutationPointAddress &address,
                                           llvm::Instruction *instruction,
                                           SourceLocation &sourceLocation) {

  std::string diagnostics;

  if (findPossibleApplication(*instruction, diagnostics) == false) {
    return nullptr;
  }

  auto mutationPoint =
    new MutationPoint(this, address, instruction, module, diagnostics, sourceLocation);

  return mutationPoint;
}

static
llvm::Value *getReplacement(Type *returnType, llvm::LLVMContext &context) {
  static const int MagicValue = 42;

  if (returnType->isIntegerTy()) {
    APInt replacementIntValue = APInt(returnType->getIntegerBitWidth(),
                                      MagicValue);

    return ConstantInt::get(context,
                            replacementIntValue);
  }
  if (returnType->isDoubleTy()) {
    APFloat replacementFloatValue = APFloat((double)MagicValue);
    return ConstantFP::get(context,
                           replacementFloatValue);
  }
  if (returnType->isFloatTy()) {
    APFloat replacementFloatValue = APFloat((float)MagicValue);
    return ConstantFP::get(context,
                           replacementFloatValue);
  }

  llvm_unreachable("Unsupported return type!");
}

llvm::Value *
ReplaceAssignmentMutator::applyMutation(Module *M,
                                                 MutationPointAddress address,
                                                 Value &_V) {

  llvm::Instruction &instruction = address.findInstruction(M);

  StoreInst *storeInstruction = dyn_cast<StoreInst>(&instruction);

  Value *storeOperand = storeInstruction->getOperand(0);
  auto returnedType = storeOperand->getType();

  Value *replacement = getReplacement(returnedType, instruction.getContext());

  storeInstruction->setOperand(0, replacement);

  return nullptr;
}

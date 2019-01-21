#include "Mutators/ReplaceCallMutator.h"

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

const std::string ReplaceCallMutator::ID = "replace_call_mutator";
const std::string ReplaceCallMutator::description = "Replaces call to a function with 42";

static bool findPossibleApplication(Value &V, std::string &outDiagnostics);

bool ReplaceCallMutator::canBeApplied(Value &V) {
  std::string diagnostics;

  return findPossibleApplication(V, diagnostics);
}

static bool findPossibleApplication(Value &V, std::string &outDiagnostics) {

  std::string diagnostics;

  Instruction *instruction = dyn_cast<Instruction>(&V);
  assert(instruction);

  unsigned opcode = instruction->getOpcode();

  if (opcode != Instruction::Call && opcode != Instruction::Invoke) {
    return false;
  }

  CallSite callSite(instruction);

  auto returnedType = callSite.getFunctionType()->getReturnType();
  if (returnedType->isIntegerTy() == false &&
      returnedType->isFloatTy()   == false &&
      returnedType->isDoubleTy()  == false) {
    return false;
  }

  if (callSite.getCalledFunction() &&
      callSite.getCalledFunction()->getName().startswith("llvm.objectsize")) {
    return false;
  }

  std::stringstream diagnosticsStream;

  diagnosticsStream << "Replace Call: replaced a call to function ";
  if (callSite.getCalledFunction() && callSite.getCalledFunction()->hasName()) {
    diagnosticsStream << callSite.getCalledFunction()->getName().str();
    diagnosticsStream << " ";
  }
  diagnosticsStream << "with 42";

  outDiagnostics.assign(diagnosticsStream.str());

  return true;
}

MutationPoint *
ReplaceCallMutator::getMutationPoint(MullModule *module,
                                     llvm::Function *function,
                                     llvm::Instruction *instruction,
                                     SourceLocation &sourceLocation,
                                     MutationPointAddress &address) {

  std::string diagnostics;

  if (findPossibleApplication(*instruction, diagnostics) == false) {
    return nullptr;
  }

  auto mutationPoint =
      new MutationPoint(this, address, instruction, function, diagnostics, sourceLocation, module);

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
ReplaceCallMutator::applyMutation(Function *function,
                                  MutationPointAddress &address) {
  llvm::Instruction &instruction = address.findInstruction(function);

  CallSite callSite(&instruction);

  auto returnedType = callSite.getFunctionType()->getReturnType();

  Value *replacement = getReplacement(returnedType, instruction.getContext());

  instruction.replaceAllUsesWith(replacement);
  instruction.eraseFromParent();

  return nullptr;
}

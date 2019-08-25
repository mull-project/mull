#include "mull/Mutators/RemoveVoidFunctionMutator.h"

#include "mull/MutationPoint.h"

#include <llvm/IR/CallSite.h>
#include <llvm/IR/DebugInfoMetadata.h>
#include <llvm/IR/DebugLoc.h>
#include <llvm/IR/InstIterator.h>
#include <llvm/IR/Module.h>

#include <fstream>
#include <iterator>
#include <sstream>

using namespace llvm;
using namespace mull;

const std::string RemoveVoidFunctionMutator::ID =
    "remove_void_function_mutator";
const std::string RemoveVoidFunctionMutator::description =
    "Removes calls to a function returning void";

std::string getDiagnostics(Instruction &instruction) {
  assert(isa<CallInst>(instruction));

  CallSite callSite = CallSite(&instruction);

  std::stringstream diagstream;

  diagstream << "Remove Void Call: removed ";
  diagstream << callSite.getCalledFunction()->getName().str();

  std::string diagnostics = diagstream.str();

  return diagnostics;
}

void RemoveVoidFunctionMutator::applyMutation(
    Function *function, const MutationPointAddress &address) {
  llvm::Instruction &I = address.findInstruction(function);

  CallInst *callInst = dyn_cast<CallInst>(&I);
  callInst->eraseFromParent();
}

std::vector<MutationPoint *>
RemoveVoidFunctionMutator::getMutations(Bitcode *bitcode,
                                        llvm::Function *function) {
  assert(bitcode);
  assert(function);

  std::vector<MutationPoint *> mutations;

  for (auto &instruction : instructions(function)) {
    auto callInstruction = dyn_cast<CallInst>(&instruction);
    if (!callInstruction) {
      continue;
    }
    auto calledFunction = callInstruction->getCalledFunction();
    if (!calledFunction ||
        calledFunction->getReturnType()->getTypeID() != Type::VoidTyID) {
      continue;
    }

    std::string diagnostics = getDiagnostics(instruction);
    std::string replacement = "🚫";

    auto point = new MutationPoint(this, &instruction, diagnostics, replacement,
                                   bitcode);
    mutations.push_back(point);
  }

  return mutations;
}

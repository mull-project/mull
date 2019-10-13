#include "mull/Mutators/RemoveVoidFunctionMutator.h"
#include "mull/MutationPoint.h"
#include <irm/irm.h>
#include <llvm/IR/CallSite.h>
#include <llvm/IR/InstIterator.h>
#include <sstream>

using namespace llvm;
using namespace mull;

const std::string RemoveVoidFunctionMutator::ID =
    "remove_void_function_mutator";
const std::string RemoveVoidFunctionMutator::description =
    "Removes calls to a function returning void";

RemoveVoidFunctionMutator::RemoveVoidFunctionMutator() : lowLevelMutators() {
  lowLevelMutators.push_back(llvm::make_unique<irm::RemoveVoidFunctionCall>());
  lowLevelMutators.push_back(
      llvm::make_unique<irm::RemoveVoidIntrinsicsCall>());
}

void RemoveVoidFunctionMutator::applyMutation(
    llvm::Function *function, const MutationPointAddress &address,
    irm::IRMutation *lowLevelMutation) {
  llvm::Instruction &instruction = address.findInstruction(function);
  lowLevelMutation->mutate(&instruction);
}

std::vector<MutationPoint *>
RemoveVoidFunctionMutator::getMutations(Bitcode *bitcode,
                                        llvm::Function *function,
                                        const mull::InstructionFilter &instructionFilter) {
  assert(bitcode);
  assert(function);

  std::vector<MutationPoint *> mutations;

  for (auto &instruction : instructions(function)) {
    for (auto &llMutation : lowLevelMutators) {
      if (llMutation->canMutate(&instruction)) {
        CallSite callSite = CallSite(&instruction);

        std::stringstream diagstream;
        diagstream << "Remove Void Call: removed ";
        if (callSite.getCalledFunction()->hasName()) {
          diagstream << callSite.getCalledFunction()->getName().str();
        }
        std::string diagnostics = diagstream.str();

        std::string replacement = "🚫";

        auto point = new MutationPoint(this, llMutation.get(), &instruction,
                                       replacement, bitcode, diagnostics);
        mutations.push_back(point);
      }
    }
  }

  return mutations;
}


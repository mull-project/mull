#include "mull/Mutators/ReplaceCallMutator.h"
#include "mull/FunctionUnderTest.h"
#include "mull/MutationPoint.h"
#include <cassert>
#include <irm/irm.h>
#include <llvm/IR/Instructions.h>
#include <sstream>

using namespace llvm;
using namespace mull;

std::string ReplaceCallMutator::ID() {
  return "replace_call_mutator";
}
std::string ReplaceCallMutator::description() {
  return "Replaces call to a function with 42";
}

ReplaceCallMutator::ReplaceCallMutator() : lowLevelMutators() {
  lowLevelMutators.push_back(std::make_unique<irm::IntCallReplacement>(42));
  lowLevelMutators.push_back(std::make_unique<irm::FloatCallReplacement>(42));
  lowLevelMutators.push_back(std::make_unique<irm::DoubleCallReplacement>(42));
}

void ReplaceCallMutator::applyMutation(llvm::Function *function,
                                       const MutationPointAddress &address,
                                       irm::IRMutation *lowLevelMutation) {
  llvm::Instruction &instruction = address.findInstruction(function);
  lowLevelMutation->mutate(&instruction);
}

std::vector<MutationPoint *> ReplaceCallMutator::getMutations(Bitcode *bitcode,
                                                              const FunctionUnderTest &function) {
  assert(bitcode);

  std::vector<MutationPoint *> mutations;

  for (llvm::Instruction *instruction : function.getSelectedInstructions()) {
    for (auto &mutator : lowLevelMutators) {
      if (mutator->canMutate(instruction)) {
        std::string replacement = "42";

        auto point = new MutationPoint(this, mutator.get(), instruction, replacement, bitcode);
        mutations.push_back(point);
      }
    }
  }

  return mutations;
}

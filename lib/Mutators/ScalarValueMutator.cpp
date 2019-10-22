#include "mull/Mutators/ScalarValueMutator.h"
#include "mull/MutationPoint.h"
#include <irm/irm.h>
#include <llvm/IR/InstIterator.h>

using namespace llvm;
using namespace mull;

const std::string ScalarValueMutator::ID = "scalar_value_mutator";
const std::string ScalarValueMutator::description =
    "Replaces zeros with 42, and non-zeros with 0";

ScalarValueMutator::ScalarValueMutator() : lowLevelMutators() {
  // i == operand position
  for (auto i = 0; i < 5; i++) {
    lowLevelMutators.push_back(
        llvm::make_unique<irm::ConstIntReplacement>(42, i));
    lowLevelMutators.push_back(
        llvm::make_unique<irm::ConstFloatReplacement>(42, i));
  }
}

void ScalarValueMutator::applyMutation(llvm::Function *function,
                                       const MutationPointAddress &address,
                                       irm::IRMutation *lowLevelMutation) {
  llvm::Instruction &instruction = address.findInstruction(function);
  lowLevelMutation->mutate(&instruction);
}

std::vector<MutationPoint *> ScalarValueMutator::getMutations(
    Bitcode *bitcode, llvm::Function *function,
    const mull::InstructionFilter &instructionFilter) {
  assert(bitcode);
  assert(function);

  std::vector<MutationPoint *> mutations;

  for (auto &instruction : instructions(function)) {
    if (!instructionFilter.validScalarMutation(instruction)) {
      continue;
    }

    for (auto &llMutation : lowLevelMutators) {
      if (llMutation->canMutate(&instruction)) {
        std::string diagnostics = "Replacing scalar with 0 or 42";
        std::string replacement = "0 or 42";

        auto point = new MutationPoint(this, llMutation.get(), &instruction,
                                       replacement, bitcode, diagnostics);
        mutations.push_back(point);
      }
    }
  }

  return mutations;
}

#include "mull/Mutators/ScalarValueMutator.h"
#include "mull/MutationPoint.h"
#include "mull/ReachableFunction.h"
#include <irm/irm.h>

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

std::vector<MutationPoint *>
ScalarValueMutator::getMutations(Bitcode *bitcode,
                                 const FunctionUnderTest &function) {
  assert(bitcode);

  std::vector<MutationPoint *> mutations;

  for (llvm::Instruction *instruction : function.getSelectedInstructions()) {
    for (auto &mutator : lowLevelMutators) {
      if (mutator->canMutate(instruction)) {
        std::string diagnostics = "Replacing scalar with 0 or 42";
        std::string replacement = "0 or 42";

        auto point = new MutationPoint(this, mutator.get(), instruction,
                                       replacement, bitcode, diagnostics);
        mutations.push_back(point);
      }
    }
  }

  return mutations;
}

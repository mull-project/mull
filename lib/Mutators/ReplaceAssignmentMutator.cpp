#include "mull/Mutators/ReplaceAssignmentMutator.h"
#include "mull/MutationPoint.h"
#include <irm/irm.h>
#include <llvm/IR/InstIterator.h>

using namespace llvm;
using namespace mull;

const std::string ReplaceAssignmentMutator::ID = "replace_assignment_mutator";
const std::string ReplaceAssignmentMutator::description =
    "Replaces assignment with 42";

ReplaceAssignmentMutator::ReplaceAssignmentMutator() : lowLevelMutators() {
  lowLevelMutators.push_back(llvm::make_unique<irm::StoreIntReplacement>(42));
  lowLevelMutators.push_back(llvm::make_unique<irm::StoreFloatReplacement>(42));
  lowLevelMutators.push_back(
      llvm::make_unique<irm::StoreDoubleReplacement>(42));
}

void ReplaceAssignmentMutator::applyMutation(
    llvm::Function *function, const MutationPointAddress &address,
    irm::IRMutation *lowLevelMutation) {
  llvm::Instruction &instruction = address.findInstruction(function);
  lowLevelMutation->mutate(&instruction);
}

std::vector<MutationPoint *>
ReplaceAssignmentMutator::getMutations(Bitcode *bitcode,
                                       llvm::Function *function) {
  assert(bitcode);
  assert(function);

  std::vector<MutationPoint *> mutations;

  for (auto &instruction : instructions(function)) {
    for (auto &llMutation : lowLevelMutators) {
      if (llMutation->canMutate(&instruction)) {
        std::string diagnostics = "Replace Assignment: replaced rvalue with 42";

        const std::string replacement = "42";

        auto point = new MutationPoint(this, llMutation.get(), &instruction,
                                       replacement, bitcode, diagnostics);
        mutations.push_back(point);
      }
    }
  }

  return mutations;
}

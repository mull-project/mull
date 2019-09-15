#include "mull/Mutators/MathAddMutator.h"
#include "mull/MutationPoint.h"

#include <irm/irm.h>
#include <llvm/IR/InstIterator.h>
#include <memory>

using namespace llvm;
using namespace mull;

const std::string MathAddMutator::ID = "math_add_mutator";
const std::string MathAddMutator::description = "Replaces + with -";

MathAddMutator::MathAddMutator() : lowLevelMutators() {
  lowLevelMutators.push_back(llvm::make_unique<irm::AddToSub>());
  lowLevelMutators.push_back(llvm::make_unique<irm::FAddToFSub>());
  lowLevelMutators.push_back(
      llvm::make_unique<irm::sadd_with_overflowTossub_with_overflow>());
}

void MathAddMutator::applyMutation(llvm::Function *function,
                                   const MutationPointAddress &address,
                                   irm::IRMutation *lowLevelMutation) {
  llvm::Instruction &instruction = address.findInstruction(function);
  lowLevelMutation->mutate(&instruction);
}

std::vector<MutationPoint *>
MathAddMutator::getMutations(Bitcode *bitcode, llvm::Function *function) {
  assert(bitcode);
  assert(function);

  std::vector<MutationPoint *> mutations;

  for (auto &instruction : instructions(function)) {
    for (auto &llMutation : lowLevelMutators) {
      if (llMutation->canMutate(&instruction)) {
        std::string diagnostics = "Math Add: replaced + with -";
        std::string replacement = "-";
        auto point = new MutationPoint(this, llMutation.get(), &instruction,
                                       replacement, bitcode, diagnostics);
        mutations.push_back(point);
      }
    }
  }

  return mutations;
}

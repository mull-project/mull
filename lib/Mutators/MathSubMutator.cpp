#include "mull/Mutators/MathSubMutator.h"
#include "mull/MutationPoint.h"
#include <irm/irm.h>
#include <llvm/IR/InstIterator.h>

using namespace llvm;
using namespace mull;

const std::string MathSubMutator::ID = "math_sub_mutator";
const std::string MathSubMutator::description = "Replaces - with +";

MathSubMutator::MathSubMutator() : lowLevelMutators() {
  lowLevelMutators.push_back(llvm::make_unique<irm::SubToAdd>());
  lowLevelMutators.push_back(llvm::make_unique<irm::FSubToFAdd>());
  lowLevelMutators.push_back(
      llvm::make_unique<irm::ssub_with_overflowTosadd_with_overflow>());
}

void MathSubMutator::applyMutation(llvm::Function *function,
                                   const MutationPointAddress &address,
                                   irm::IRMutation *lowLevelMutation) {
  llvm::Instruction &instruction = address.findInstruction(function);
  lowLevelMutation->mutate(&instruction);
}

std::vector<MutationPoint *>
MathSubMutator::getMutations(Bitcode *bitcode, llvm::Function *function) {
  assert(bitcode);
  assert(function);

  std::vector<MutationPoint *> mutations;

  for (auto &instruction : instructions(function)) {
    for (auto &llMutation : lowLevelMutators) {
      if (llMutation->canMutate(&instruction)) {
        std::string diagnostics = "Math Sub: replaced - with +";
        std::string replacement = "+";
        auto point = new MutationPoint(this, llMutation.get(), &instruction,
                                       replacement, bitcode, diagnostics);
        mutations.push_back(point);
      }
    }
  }

  return mutations;
}

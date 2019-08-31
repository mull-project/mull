#include "mull/Mutators/MathDivMutator.h"
#include "mull/MutationPoint.h"
#include <irm/irm.h>
#include <llvm/IR/InstIterator.h>

using namespace llvm;
using namespace mull;

const std::string MathDivMutator::ID = "math_div_mutator";
const std::string MathDivMutator::description = "Replaces / with *";

MathDivMutator::MathDivMutator() : lowLevelMutators() {
  lowLevelMutators.push_back(llvm::make_unique<irm::UDivToMul>());
  lowLevelMutators.push_back(llvm::make_unique<irm::SDivToMul>());
  lowLevelMutators.push_back(llvm::make_unique<irm::FDivToFMul>());
}

void MathDivMutator::applyMutation(llvm::Function *function,
                                   const MutationPointAddress &address,
                                   irm::IRMutation *lowLevelMutation) {
  llvm::Instruction &instruction = address.findInstruction(function);
  lowLevelMutation->mutate(&instruction);
}

std::vector<MutationPoint *>
MathDivMutator::getMutations(Bitcode *bitcode, llvm::Function *function) {
  assert(bitcode);
  assert(function);

  std::vector<MutationPoint *> mutations;

  for (auto &instruction : instructions(function)) {
    for (auto &llMutation : lowLevelMutators) {
      if (llMutation->canMutate(&instruction)) {
        std::string diagnostics = "Math Div: replaced / with *";
        std::string replacement = "*";
        auto point = new MutationPoint(this, llMutation.get(), &instruction,
                                       replacement, bitcode, diagnostics);
        mutations.push_back(point);
      }
    }
  }

  return mutations;
}

#include "mull/Mutators/MathMulMutator.h"
#include "mull/MutationPoint.h"
#include <irm/irm.h>
#include <llvm/IR/InstIterator.h>

using namespace llvm;
using namespace mull;

const std::string MathMulMutator::ID = "math_mul_mutator";
const std::string MathMulMutator::description = "Replaces * with /";

MathMulMutator::MathMulMutator() : lowLevelMutators() {
  lowLevelMutators.push_back(llvm::make_unique<irm::MulToSDiv>());
  lowLevelMutators.push_back(llvm::make_unique<irm::FMulToFDiv>());
}

void MathMulMutator::applyMutation(llvm::Function *function,
                                   const MutationPointAddress &address,
                                   irm::IRMutation *lowLevelMutation) {
  llvm::Instruction &instruction = address.findInstruction(function);
  lowLevelMutation->mutate(&instruction);
}

std::vector<MutationPoint *>
MathMulMutator::getMutations(Bitcode *bitcode, llvm::Function *function) {
  assert(bitcode);
  assert(function);

  std::vector<MutationPoint *> mutations;

  for (auto &instruction : instructions(function)) {
    for (auto &llMutation : lowLevelMutators) {
      if (llMutation->canMutate(&instruction)) {
        std::string diagnostics = "Math Mul: replaced * with /";
        std::string replacement = "/";
        auto point = new MutationPoint(this, llMutation.get(), &instruction,
                                       replacement, bitcode, diagnostics);
        mutations.push_back(point);
      }
    }
  }
  return mutations;
}

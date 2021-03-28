#include "mull/Mutators/RemoveVoidFunctionMutator.h"
#include "mull/FunctionUnderTest.h"
#include "mull/MutationPoint.h"
#include <irm/irm.h>
#include <llvm/IR/Instructions.h>
#include <sstream>

using namespace llvm;
using namespace mull;

std::string RemoveVoidFunctionMutator::ID() {
  return "remove_void_function_mutator";
}
std::string RemoveVoidFunctionMutator::description() {
  return "Removes calls to a function returning void";
}

RemoveVoidFunctionMutator::RemoveVoidFunctionMutator() : lowLevelMutators() {
  lowLevelMutators.push_back(std::make_unique<irm::RemoveVoidFunctionCall>());
  lowLevelMutators.push_back(std::make_unique<irm::RemoveVoidIntrinsicsCall>());
}

void RemoveVoidFunctionMutator::applyMutation(llvm::Function *function,
                                              const MutationPointAddress &address,
                                              irm::IRMutation *lowLevelMutation) {
  llvm::Instruction &instruction = address.findInstruction(function);
  lowLevelMutation->mutate(&instruction);
}

std::vector<MutationPoint *>
RemoveVoidFunctionMutator::getMutations(Bitcode *bitcode, const FunctionUnderTest &function) {
  assert(bitcode);

  std::vector<MutationPoint *> mutations;

  for (llvm::Instruction *instruction : function.getSelectedInstructions()) {
    for (auto &mutator : lowLevelMutators) {
      if (mutator->canMutate(instruction)) {
        std::string replacement = "🚫";

        auto point = new MutationPoint(this, mutator.get(), instruction, replacement, bitcode);
        mutations.push_back(point);
      }
    }
  }

  return mutations;
}

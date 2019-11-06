#include "mull/Mutators/RemoveVoidFunctionMutator.h"
#include "mull/MutationPoint.h"
#include "mull/ReachableFunction.h"
#include <irm/irm.h>
#include <llvm/IR/CallSite.h>
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
                                        const FunctionUnderTest &function) {
  assert(bitcode);

  std::vector<MutationPoint *> mutations;

  for (llvm::Instruction *instruction : function.getSelectedInstructions()) {
    for (auto &mutator : lowLevelMutators) {
      if (mutator->canMutate(instruction)) {
        CallSite callSite = CallSite(instruction);

        std::stringstream diagstream;
        diagstream << "Remove Void Call: removed ";
        if (callSite.getCalledFunction()->hasName()) {
          diagstream << callSite.getCalledFunction()->getName().str();
        }
        std::string diagnostics = diagstream.str();

        std::string replacement = "🚫";

        auto point = new MutationPoint(this, mutator.get(), instruction,
                                       replacement, bitcode, diagnostics);
        mutations.push_back(point);
      }
    }
  }

  return mutations;
}

#include "mull/Mutators/ReplaceCallMutator.h"
#include "mull/MutationPoint.h"
#include "mull/ReachableFunction.h"
#include <irm/irm.h>
#include <llvm/IR/CallSite.h>
#include <llvm/IR/InstIterator.h>
#include <mull/ReachableFunction.h>
#include <sstream>

using namespace llvm;
using namespace mull;

const std::string ReplaceCallMutator::ID = "replace_call_mutator";
const std::string ReplaceCallMutator::description =
    "Replaces call to a function with 42";

ReplaceCallMutator::ReplaceCallMutator() : lowLevelMutators() {
  lowLevelMutators.push_back(llvm::make_unique<irm::IntCallReplacement>(42));
  lowLevelMutators.push_back(llvm::make_unique<irm::FloatCallReplacement>(42));
  lowLevelMutators.push_back(llvm::make_unique<irm::DoubleCallReplacement>(42));
}

void ReplaceCallMutator::applyMutation(llvm::Function *function,
                                       const MutationPointAddress &address,
                                       irm::IRMutation *lowLevelMutation) {
  llvm::Instruction &instruction = address.findInstruction(function);
  lowLevelMutation->mutate(&instruction);
}

std::vector<MutationPoint *>
ReplaceCallMutator::getMutations(Bitcode *bitcode,
                                 const FunctionUnderTest &function) {
  assert(bitcode);

  std::vector<MutationPoint *> mutations;

  for (auto &instruction : instructions(function.getFunction())) {
    for (auto &llMutation : lowLevelMutators) {
      if (llMutation->canMutate(&instruction)) {
        CallSite callSite(&instruction);

        std::stringstream diagnostics;
        diagnostics << "Replace Call: replaced a call to function ";
        if (callSite.getCalledFunction()->hasName()) {
          diagnostics << callSite.getCalledFunction()->getName().str();
        }
        diagnostics << " with 42";

        std::string replacement = "42";

        auto point = new MutationPoint(this, llMutation.get(), &instruction,
                                       replacement, bitcode, diagnostics.str());
        mutations.push_back(point);
      }
    }
  }

  return mutations;
}

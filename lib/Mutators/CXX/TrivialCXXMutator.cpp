#include "mull/Mutators/CXX/TrivialCXXMutator.h"
#include "mull/FunctionUnderTest.h"
#include "mull/MutationPoint.h"
#include <irm/irm.h>

using namespace mull;
using namespace mull::cxx;

TrivialCXXMutator::TrivialCXXMutator(std::vector<std::unique_ptr<irm::IRMutation>> mutators,
                                     MutatorKind kind, std::string id, std::string description,
                                     std::string replacement, std::string diagnostics)
    : lowLevelMutators(std::move(mutators)), kind(kind), ID(std::move(id)),
      description(std::move(description)), replacement(std::move(replacement)),
      diagnostics(std::move(diagnostics)) {}

std::string TrivialCXXMutator::getUniqueIdentifier() {
  return ID;
}

std::string TrivialCXXMutator::getUniqueIdentifier() const {
  return ID;
}

std::string TrivialCXXMutator::getDescription() const {
  return description;
}

std::string TrivialCXXMutator::getDiagnostics() const {
  return diagnostics;
}

std::string TrivialCXXMutator::getReplacement() const {
  return replacement;
}

MutatorKind TrivialCXXMutator::mutatorKind() {
  return kind;
}

void TrivialCXXMutator::applyMutation(llvm::Function *function, const MutationPointAddress &address,
                                      irm::IRMutation *lowLevelMutation) {
  llvm::Instruction &instruction = address.findInstruction(function);
  lowLevelMutation->mutate(&instruction);
}

std::vector<MutationPoint *> TrivialCXXMutator::getMutations(Bitcode *bitcode,
                                                             const FunctionUnderTest &function) {
  assert(bitcode);

  std::vector<MutationPoint *> mutations;

  for (llvm::Instruction *instruction : function.getSelectedInstructions()) {
    for (auto &mutator : lowLevelMutators) {
      if (mutator->canMutate(instruction)) {
        auto point = new MutationPoint(this, mutator.get(), instruction, bitcode);
        mutations.push_back(point);
      }
    }
  }

  return mutations;
}

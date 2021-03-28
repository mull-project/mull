#pragma once

#include "Mutator.h"
#include <irm/irm.h>
#include <memory>
#include <mull/FunctionUnderTest.h>
#include <vector>

namespace llvm {
class Instruction;
}

namespace mull {

class Bitcode;
class MutationPoint;
class MutationPointAddress;
class FunctionUnderTest;

class NegateConditionMutator : public Mutator {
public:
  static std::string ID();
  static std::string description();

  NegateConditionMutator();

  MutatorKind mutatorKind() override { return MutatorKind::NegateMutator; }
  std::string getUniqueIdentifier() override {
    return ID();
  }
  std::string getUniqueIdentifier() const override {
    return ID();
  }
  std::string getDescription() const override {
    return description();
  }
  std::string getDiagnostics() const override {
    return "Negated !x to x (or x to !x)";
  }
  std::string getReplacement() const override {
    return "x or !x";
  }

  void applyMutation(llvm::Function *function,
                     const MutationPointAddress &address,
                     irm::IRMutation *lowLevelMutation) override;

  std::vector<MutationPoint *>
  getMutations(Bitcode *bitcode, const FunctionUnderTest &function) override;

private:
  std::vector<std::unique_ptr<irm::IRMutation>> lowLevelMutators;
};
} // namespace mull

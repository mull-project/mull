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

class ScalarValueMutator : public Mutator {

public:
  static std::string ID();
  static std::string description();

  ScalarValueMutator();

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
    return "Replaced scalar with 0 or 42";
  }
  std::string getReplacement() const override {
    return "0 or 42";
  }
  MutatorKind mutatorKind() override { return MutatorKind::ScalarValueMutator; }

  void applyMutation(llvm::Function *function,
                     const MutationPointAddress &address,
                     irm::IRMutation *lowLevelMutation) override;

  std::vector<MutationPoint *>
  getMutations(Bitcode *bitcode, const FunctionUnderTest &function) override;

private:
  std::vector<std::unique_ptr<irm::IRMutation>> lowLevelMutators;
};

} // namespace mull

#pragma once

#include "Mutator.h"
#include <irm/irm.h>
#include <memory>
#include <mull/FunctionUnderTest.h>
#include <vector>

namespace mull {

class MutationPoint;
class MutationPointAddress;
class FunctionUnderTest;

class ReplaceCallMutator : public Mutator {

public:
  static std::string ID();
  static std::string description();

  ReplaceCallMutator();

  std::string getUniqueIdentifier() override {
    return ID();
  }
  std::string getUniqueIdentifier() const override {
    return ID();
  }
  std::string getDescription() const override {
    return description();
  }
  MutatorKind mutatorKind() override {
    return MutatorKind::ReplaceCallMutator;
  }

  void applyMutation(llvm::Function *function, const MutationPointAddress &address,
                     irm::IRMutation *lowLevelMutation) override;

  std::vector<MutationPoint *> getMutations(Bitcode *bitcode,
                                            const FunctionUnderTest &function) override;

private:
  std::vector<std::unique_ptr<irm::IRMutation>> lowLevelMutators;
};

} // namespace mull

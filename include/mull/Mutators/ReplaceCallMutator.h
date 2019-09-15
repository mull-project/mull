#pragma once

#include "Mutator.h"
#include <irm/irm.h>
#include <memory>
#include <vector>

namespace mull {

class MutationPoint;
class MutationPointAddress;

class ReplaceCallMutator : public Mutator {

public:
  static const std::string ID;
  static const std::string description;

  ReplaceCallMutator();

  std::string getUniqueIdentifier() override { return ID; }
  std::string getUniqueIdentifier() const override { return ID; }
  std::string getDescription() const override { return description; }
  MutatorKind mutatorKind() override { return MutatorKind::ReplaceCallMutator; }

  void applyMutation(llvm::Function *function,
                     const MutationPointAddress &address,
                     irm::IRMutation *lowLevelMutation) override;

  std::vector<MutationPoint *> getMutations(Bitcode *bitcode,
                                            llvm::Function *function) override;

private:
  std::vector<std::unique_ptr<irm::IRMutation>> lowLevelMutators;
};

} // namespace mull

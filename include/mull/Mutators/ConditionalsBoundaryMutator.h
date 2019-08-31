#pragma once

#include "Mutator.h"
#include <irm/irm.h>
#include <memory>
#include <vector>

namespace llvm {
class Instruction;
}

namespace mull {

class Bitcode;
class MutationPoint;
class MutationPointAddress;

class ConditionalsBoundaryMutator : public Mutator {

public:
  static const std::string ID;
  static const std::string description;

  ConditionalsBoundaryMutator();

  std::string getUniqueIdentifier() override;
  std::string getUniqueIdentifier() const override;
  std::string getDescription() const override;
  MutatorKind mutatorKind() override;

  void applyMutation(llvm::Function *function,
                     const MutationPointAddress &address,
                     irm::IRMutation *lowLevelMutation) override;

  std::vector<MutationPoint *> getMutations(Bitcode *bitcode,
                                            llvm::Function *function) override;

private:
  std::vector<std::unique_ptr<irm::IRMutation>> lowLevelMutators;
};

} // namespace mull

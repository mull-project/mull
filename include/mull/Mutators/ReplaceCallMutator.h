#pragma once

#include "Mutator.h"

#include <vector>

namespace mull {

class MutationPoint;
class MutationPointAddress;

class ReplaceCallMutator : public Mutator {

public:
  static const std::string ID;
  static const std::string description;

  std::string getUniqueIdentifier() override { return ID; }
  std::string getUniqueIdentifier() const override { return ID; }
  std::string getDescription() const override { return description; }
  MutatorKind mutatorKind() override { return MutatorKind::ReplaceCallMutator; }

  void applyMutation(llvm::Function *function,
                     const MutationPointAddress &address) override;

  std::vector<MutationPoint *> getMutations(Bitcode *bitcode,
                                            llvm::Function *function) override;
};

} // namespace mull

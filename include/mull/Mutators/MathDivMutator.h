#pragma once

#include "Mutator.h"

#include <vector>

namespace llvm {
class Instruction;
}

namespace mull {

class Bitcode;
class MutationPoint;
class MutationPointAddress;

class MathDivMutator : public Mutator {

public:
  static const std::string ID;
  static const std::string description;

  MutationPoint *getMutationPoint(Bitcode *bitcode, llvm::Function *function,
                                  llvm::Instruction *instruction,
                                  SourceLocation &sourceLocation,
                                  MutationPointAddress &address) override;

  std::string getUniqueIdentifier() override { return ID; }
  std::string getUniqueIdentifier() const override { return ID; }
  std::string getDescription() const override { return description; }
  MutatorKind mutatorKind() override { return MutatorKind::MathDivMutator; }

  bool canBeApplied(llvm::Value &V) override;
  llvm::Value *applyMutation(llvm::Function *function,
                             MutationPointAddress &address) override;
};

} // namespace mull

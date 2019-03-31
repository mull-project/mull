#pragma once

#include "Mutator.h"

#include <vector>

namespace mull {

class MutationPoint;
class MutationPointAddress;

/// TODO: Move Add With Overflow Mutation to a separate mutator.
/// Arithmetic with Overflow Intrinsics
/// http://llvm.org/docs/LangRef.html#id1468
class MathAddMutator : public Mutator {

  bool isAddWithOverflow(llvm::Value &V);
  llvm::Function *replacementForAddWithOverflow(llvm::Function *addFunction,
                                                llvm::Module &module);

public:
  static const std::string ID;
  static const std::string description;

  MutationPoint *getMutationPoint(MullModule *module, llvm::Function *function,
                                  llvm::Instruction *instruction,
                                  SourceLocation &sourceLocation,
                                  MutationPointAddress &address) override;
  MutatorKind mutatorKind() override { return MutatorKind::MathAddMutator; }

  std::string getUniqueIdentifier() override { return ID; }
  std::string getUniqueIdentifier() const override { return ID; }
  std::string getDescription() const override { return description; }

  bool canBeApplied(llvm::Value &V) override;
  llvm::Value *applyMutation(llvm::Function *function,
                             MutationPointAddress &address) override;
};

} // namespace mull

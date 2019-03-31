#pragma once

#include "Mutator.h"

#include <vector>

namespace llvm {
class Instruction;
}

namespace mull {

class MullModule;
class MutationPoint;
class MutationPointAddress;

/// Arithmetic with Overflow Intrinsics
/// http://llvm.org/docs/LangRef.html#id1468
class MathSubMutator : public Mutator {

  bool isSubWithOverflow(llvm::Value &V);
  llvm::Function *replacementForSubWithOverflow(llvm::Function *testeeFunction,
                                                llvm::Module &module);

public:
  static const std::string ID;
  static const std::string description;

  MutationPoint *getMutationPoint(MullModule *module, llvm::Function *function,
                                  llvm::Instruction *instruction,
                                  SourceLocation &sourceLocation,
                                  MutationPointAddress &address) override;

  std::string getUniqueIdentifier() override { return ID; }
  std::string getUniqueIdentifier() const override { return ID; }
  std::string getDescription() const override { return description; }
  MutatorKind mutatorKind() override { return MutatorKind::MathSubMutator; }

  bool canBeApplied(llvm::Value &V) override;
  llvm::Value *applyMutation(llvm::Function *function,
                             MutationPointAddress &address) override;
};

} // namespace mull

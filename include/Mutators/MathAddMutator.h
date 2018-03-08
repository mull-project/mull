#pragma once

#include "Mutators/Mutator.h"

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

  MutationPoint *getMutationPoint(MullModule *module,
                                  MutationPointAddress &address,
                                  llvm::Instruction *instruction) override;
  MutatorKind mutatorKind() override { return MutatorKind::MathAddMutator; }

  std::string uniqueID() override {
    return ID;
  }
  std::string uniqueID() const override {
    return ID;
  }

  bool canBeApplied(llvm::Value &V) override;
  llvm::Value *applyMutation(llvm::Module *M, MutationPointAddress address, llvm::Value &OriginalValue) override;
};

}

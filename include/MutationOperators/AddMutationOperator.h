#pragma once

#include "MutationOperators/MutationOperator.h"

#include <vector>

namespace mull {

class MutationPoint;
class MutationPointAddress;
class MutationOperatorFilter;

/// Arithmetic with Overflow Intrinsics
/// http://llvm.org/docs/LangRef.html#id1468
class AddMutationOperator : public MutationOperator {

  bool isAddWithOverflow(llvm::Value &V);
  llvm::Function *replacementForAddWithOverflow(llvm::Function *addFunction,
                                                llvm::Module &module);

public:
  static const std::string ID;

  std::vector<MutationPoint *> getMutationPoints(const Context &context,
                                                 llvm::Function *function,
                                                 MutationOperatorFilter &filter) override;

  std::string uniqueID() override {
    return ID;
  }
  std::string uniqueID() const override {
    return ID;
  }

  bool canBeApplied(llvm::Value &V) override;
  llvm::Value *applyMutation(llvm::Module *M, MutationPointAddress address, llvm::Value &OriginalValue) override;
  llvm::Value *revertMutation(llvm::Value &Value) override __attribute__((unavailable));
};

}


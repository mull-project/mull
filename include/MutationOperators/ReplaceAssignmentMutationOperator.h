#pragma once

#include "MutationOperators/Mutator.h"

#include <vector>

namespace mull {

class MutationPoint;
class MutationPointAddress;
class MutationOperatorFilter;

class ReplaceAssignmentMutationOperator : public Mutator {

public:
  static const std::string ID;

  MutationPoint *getMutationPoint(MullModule *module,
                                  MutationPointAddress &address,
                                  llvm::Instruction *instruction) override;

  std::string uniqueID() override {
    return ID;
  }
  std::string uniqueID() const override {
    return ID;
  }

  bool canBeApplied(llvm::Value &V) override;
  llvm::Value *applyMutation(llvm::Module *M,
                             MutationPointAddress address,
                             llvm::Value &OriginalValue) override;
};

}

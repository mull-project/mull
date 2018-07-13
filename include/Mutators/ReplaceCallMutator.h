#pragma once

#include "Mutators/Mutator.h"

#include <vector>

namespace mull {

class MutationPoint;
class MutationPointAddress;

class ReplaceCallMutator : public Mutator {

public:
  static const std::string ID;

  MutationPoint *getMutationPoint(MullModule *module,
                                    MutationPointAddress &address,
                                    llvm::Instruction *instruction,
                                    SourceLocation &sourceLocation) override;

  std::string getUniqueIdentifier() override {
    return ID;
  }
  std::string getUniqueIdentifier() const override {
    return ID;
  }

  bool canBeApplied(llvm::Value &V) override;
  llvm::Value *
  applyMutation(llvm::Module *module, MutationPointAddress &address) override;
};

}

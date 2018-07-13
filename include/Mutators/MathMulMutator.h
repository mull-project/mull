#pragma once

#include "Mutators/Mutator.h"

#include <vector>

namespace llvm {
class Instruction;
}

namespace mull {

class MullModule;
class MutationPoint;
class MutationPointAddress;

class MathMulMutator : public Mutator {

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

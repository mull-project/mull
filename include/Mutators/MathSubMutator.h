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

/// Arithmetic with Overflow Intrinsics
/// http://llvm.org/docs/LangRef.html#id1468
class MathSubMutator : public Mutator {
  
  bool isSubWithOverflow(llvm::Value &V);
  llvm::Function *replacementForSubWithOverflow(llvm::Function *testeeFunction,
                                                llvm::Module &module);

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

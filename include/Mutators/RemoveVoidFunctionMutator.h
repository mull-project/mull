#pragma once

#include "Mutators/Mutator.h"

#include <llvm/IR/Instructions.h>

#include <vector>

namespace llvm {
  class Instruction;
}

namespace mull {

  class MullModule;
  class MutationPoint;
  class MutationPointAddress;

  class RemoveVoidFunctionMutator : public Mutator {
    
  public:
    static const std::string ID;

    MutationPoint *getMutationPoint(MullModule *module,
                                        llvm::Function *function,
                                        llvm::Instruction *instruction,
                                        SourceLocation &sourceLocation,
                                        MutationPointAddress &address) override;

    MutatorKind mutatorKind() override { return MutatorKind::RemoveVoidFunctionMutator; }

    std::string getUniqueIdentifier() override {
      return ID;
    }
    std::string getUniqueIdentifier() const override {
      return ID;
    }

    bool canBeApplied(llvm::Value &V) override;
    llvm::Value *
    applyMutation(llvm::Function *function, MutationPointAddress &address) override;
  };
}

#pragma once

#include "Mutator.h"

#include <llvm/IR/Instructions.h>

#include <vector>

namespace llvm {
class Instruction;
}

namespace mull {

class MullModule;
class MutationPoint;
class MutationPointAddress;

class ConditionalsBoundaryMutator : public Mutator {

public:
  static const std::string ID;
  static const std::string description;
  std::string getUniqueIdentifier() override;
  std::string getUniqueIdentifier() const override;
  std::string getDescription() const override;
  MutatorKind mutatorKind() override;

  static bool isGT(llvm::Instruction *instruction);
  static bool isGTE(llvm::Instruction *instruction);
  static bool isLT(llvm::Instruction *instruction);
  static bool isLTE(llvm::Instruction *instruction);

  MutationPoint *getMutationPoint(MullModule *module, llvm::Function *function,
                                  llvm::Instruction *instruction,
                                  SourceLocation &sourceLocation,
                                  MutationPointAddress &address) override;

  bool canBeApplied(llvm::Value &V) override;
  llvm::Value *applyMutation(llvm::Function *function,
                             MutationPointAddress &address) override;
};

} // namespace mull

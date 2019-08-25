#pragma once

#include "Mutator.h"

#include <llvm/IR/Instructions.h>

#include <vector>

namespace llvm {
class Instruction;
}

namespace mull {

class Bitcode;
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

  void applyMutation(llvm::Function *function,
                     const MutationPointAddress &address) override;

  std::vector<MutationPoint *> getMutations(Bitcode *bitcode,
                                            llvm::Function *function) override;
};

} // namespace mull

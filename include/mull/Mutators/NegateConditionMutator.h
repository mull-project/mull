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

class NegateConditionMutator : public Mutator {

public:
  static const std::string ID;
  static const std::string description;

  static llvm::CmpInst::Predicate
  negatedCmpInstPredicate(llvm::CmpInst::Predicate predicate);

  MutatorKind mutatorKind() override { return MutatorKind::NegateMutator; }
  std::string getUniqueIdentifier() override { return ID; }
  std::string getUniqueIdentifier() const override { return ID; }
  std::string getDescription() const override { return description; }

  void applyMutation(llvm::Function *function,
                     const MutationPointAddress &address) override;

  std::vector<MutationPoint *> getMutations(Bitcode *bitcode,
                                            llvm::Function *function) override;
};
} // namespace mull

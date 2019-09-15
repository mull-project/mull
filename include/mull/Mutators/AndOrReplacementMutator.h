#pragma once

#include "Mutator.h"

#include <vector>

enum AND_OR_MutationType {
  AND_OR_MutationType_None = 0,

  AND_OR_MutationType_AND_to_OR_Pattern1 = 11,
  AND_OR_MutationType_AND_to_OR_Pattern2 = 12,
  AND_OR_MutationType_AND_to_OR_Pattern3 = 13,

  AND_OR_MutationType_OR_to_AND_Pattern1 = 21,
  AND_OR_MutationType_OR_to_AND_Pattern2 = 22,
  AND_OR_MutationType_OR_to_AND_Pattern3 = 23
};

namespace llvm {
class BasicBlock;
class BranchInst;
class Instruction;
} // namespace llvm

using namespace llvm;

namespace mull {

class Bitcode;
class MutationPoint;
class MutationPointAddress;

class AndOrReplacementMutator : public Mutator {

  AND_OR_MutationType findPossibleMutation(Value &V);

  AND_OR_MutationType
  findPossibleMutationInBranch(BranchInst *branchInst,
                               BranchInst **secondBranchInst);

  // AND -> OR
  void applyMutationANDToOR_Pattern1(BranchInst *firstBranch,
                                     BranchInst *secondBranch);
  void applyMutationANDToOR_Pattern2(BranchInst *firstBranch,
                                     BranchInst *secondBranch);
  void applyMutationANDToOR_Pattern3(BranchInst *firstBranch,
                                     BranchInst *secondBranch);

  // OR -> AND
  void applyMutationORToAND_Pattern1(BranchInst *firstBranch,
                                     BranchInst *secondBranch);
  void applyMutationORToAND_Pattern2(BranchInst *firstBranch,
                                     BranchInst *secondBranch);
  void applyMutationORToAND_Pattern3(BranchInst *firstBranch,
                                     BranchInst *secondBranch);

public:
  static const std::string ID;
  static const std::string description;

  std::string getUniqueIdentifier() override { return ID; }
  std::string getUniqueIdentifier() const override { return ID; }
  std::string getDescription() const override { return description; }
  MutatorKind mutatorKind() override {
    return MutatorKind::AndOrReplacementMutator;
  }

  void applyMutation(llvm::Function *function,
                     const MutationPointAddress &address,
                     irm::IRMutation *lowLevelMutation) override;

  std::vector<MutationPoint *> getMutations(Bitcode *bitcode,
                                            llvm::Function *function) override;
};
} // namespace mull

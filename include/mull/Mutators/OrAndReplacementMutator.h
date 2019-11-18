#pragma once

#include "Mutator.h"
#include <vector>

enum OR_AND_MutationType {
  OR_AND_MutationType_None = 0,

  OR_AND_MutationType_OR_to_AND_Pattern1 = 21,
  OR_AND_MutationType_OR_to_AND_Pattern2 = 22,
  OR_AND_MutationType_OR_to_AND_Pattern3 = 23
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
class ReachableFunction;

class OrAndReplacementMutator : public Mutator {

  OR_AND_MutationType findPossibleMutation(Value &V);

  OR_AND_MutationType
  findPossibleMutationInBranch(BranchInst *branchInst,
                               BranchInst **secondBranchInst);

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
    return MutatorKind::CXX_OrAndReplacementMutator;
  }

  void applyMutation(llvm::Function *function,
                     const MutationPointAddress &address,
                     irm::IRMutation *lowLevelMutation) override;

  std::vector<MutationPoint *>
  getMutations(Bitcode *bitcode, const FunctionUnderTest &function) override;
};
} // namespace mull

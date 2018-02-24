#pragma once

#include "MutationOperators/Mutator.h"

#include <vector>

enum AND_OR_MutationType {
  AND_OR_MutationType_None               = 0,

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
}

using namespace llvm;

namespace mull {

  class MullModule;
  class MutationPoint;
  class MutationPointAddress;

  class AndOrReplacementMutator : public Mutator {

    AND_OR_MutationType findPossibleMutationInBranch(BranchInst *branchInst,
                                                     BranchInst **secondBranchInst);

    // AND -> OR
    llvm::Value *applyMutationANDToOR_Pattern1(Module *M,
                                               BranchInst *firstBranch,
                                               BranchInst *secondBranch);
    llvm::Value *applyMutationANDToOR_Pattern2(Module *M,
                                               BranchInst *firstBranch,
                                               BranchInst *secondBranch);
    llvm::Value *applyMutationANDToOR_Pattern3(Module *M,
                                               BranchInst *firstBranch,
                                               BranchInst *secondBranch);

    // OR -> AND
    llvm::Value *applyMutationORToAND_Pattern1(Module *M,
                                               BranchInst *firstBranch,
                                               BranchInst *secondBranch);
    llvm::Value *applyMutationORToAND_Pattern2(Module *M,
                                               BranchInst *firstBranch,
                                               BranchInst *secondBranch);
    llvm::Value *applyMutationORToAND_Pattern3(Module *M,
                                               BranchInst *firstBranch,
                                               BranchInst *secondBranch);

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
    llvm::Value *applyMutation(llvm::Module *M, MutationPointAddress address, llvm::Value &OriginalValue) override;
  };
}

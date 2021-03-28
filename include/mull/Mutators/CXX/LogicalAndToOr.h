#pragma once

#include "mull/Mutators/Mutator.h"
#include <vector>

enum AND_OR_MutationType {
  AND_OR_MutationType_None = 0,

  AND_OR_MutationType_AND_to_OR_Pattern1 = 11,
  AND_OR_MutationType_AND_to_OR_Pattern2 = 12,
  AND_OR_MutationType_AND_to_OR_Pattern3 = 13,
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
namespace cxx {

class LogicalAndToOr : public Mutator {

  static AND_OR_MutationType findPossibleMutation(Value &V);

  static AND_OR_MutationType findPossibleMutationInBranch(BranchInst *branchInst,
                                                          BranchInst **secondBranchInst);

  static void applyMutationANDToOR_Pattern1(BranchInst *firstBranch, BranchInst *secondBranch);
  static void applyMutationANDToOR_Pattern2(BranchInst *firstBranch, BranchInst *secondBranch);
  static void applyMutationANDToOR_Pattern3(BranchInst *firstBranch, BranchInst *secondBranch);

public:
  static std::string ID();
  static std::string description();

  std::string getUniqueIdentifier() override {
    return ID();
  }
  std::string getUniqueIdentifier() const override {
    return ID();
  }
  std::string getDescription() const override {
    return description();
  }
  std::string getDiagnostics() const override {
    return "Replaced && with ||";
  }
  std::string getReplacement() const override {
    return "||";
  }
  MutatorKind mutatorKind() override {
    return MutatorKind::CXX_Logical_AndToOr;
  }

  void applyMutation(llvm::Function *function, const MutationPointAddress &address,
                     irm::IRMutation *lowLevelMutation) override;

  std::vector<MutationPoint *> getMutations(Bitcode *bitcode,
                                            const FunctionUnderTest &function) override;
};

} // namespace cxx
} // namespace mull

#include "MutationOperators/AndOrReplacementMutationOperator.h"

#include "Context.h"
#include "Logger.h"
#include "MutationOperators/MutationOperatorFilter.h"
#include "MutationPoint.h"

#include "llvm/IR/InstIterator.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/DebugLoc.h"
#include "llvm/IR/DebugInfoMetadata.h"

#include <fstream>
#include <iterator>

using namespace llvm;
using namespace mull;

const std::string AndOrReplacementMutationOperator::ID = "add_or_replacement_mutation_operator";

static int GetFunctionIndex(llvm::Function *function) {
  auto PM = function->getParent();

  auto FII = std::find_if(PM->begin(), PM->end(),
                          [function] (llvm::Function &f) {
                            return &f == function;
                          });

  assert(FII != PM->end() && "Expected function to be found in module");
  int FIndex = std::distance(PM->begin(), FII);

  return FIndex;
}

std::vector<MutationPoint *>
AndOrReplacementMutationOperator::getMutationPoints(const Context &context,
                                                    llvm::Function *function,
                                                    MutationOperatorFilter &filter) {
  int functionIndex = GetFunctionIndex(function);
  int basicBlockIndex = 0;

  std::vector<MutationPoint *> mutationPoints;

  for (auto &basicBlock : function->getBasicBlockList()) {
    int instructionIndex = 0;

    for (auto &instruction : basicBlock.getInstList()) {

      if (canBeApplied(instruction) && !filter.shouldSkipInstruction(&instruction)) {
        auto moduleID = instruction.getModule()->getModuleIdentifier();
        MullModule *module = context.moduleWithIdentifier(moduleID);

        MutationPointAddress address(functionIndex, basicBlockIndex, instructionIndex);
        auto mutationPoint = new MutationPoint(this, address, &instruction, module);
        mutationPoints.push_back(mutationPoint);
      }
      instructionIndex++;
    }
    basicBlockIndex++;
  }

  return mutationPoints;
}

bool AndOrReplacementMutationOperator::canBeApplied(Value &V) {
  BranchInst *branchInst = dyn_cast<BranchInst>(&V);
  if (branchInst == nullptr || branchInst->isConditional() == false) {
    return false;
  }

  AND_OR_MutationType possibleMutationType =
    findPossibleMutationInBranch(branchInst,
                                 dyn_cast<BasicBlock>(branchInst->getOperand(2)),
                                 dyn_cast<BasicBlock>(branchInst->getOperand(1)));

  return possibleMutationType != AND_OR_MutationType_None;
}

llvm::Value *AndOrReplacementMutationOperator::applyMutation(Module *M,
                                                             MutationPointAddress address,
                                                             Value &_V) {
  /// In the following V argument is not used. Eventually it will be removed from
  /// this method's signature because it will be not relevant
  /// when mutations will be applied on copies of original module
  llvm::Function &F = *(std::next(M->begin(), address.getFnIndex()));
  llvm::BasicBlock &B = *(std::next(F.begin(), address.getBBIndex()));
  llvm::Instruction &I = *(std::next(B.begin(), address.getIIndex()));

  BranchInst *branchInst = dyn_cast<BranchInst>(&I);
  assert(branchInst != nullptr);
  assert(branchInst->isConditional());

  /// Operand #0 is a comparison instruction.
  CmpInst *cmpInst = (dyn_cast<CmpInst>(branchInst->getOperand(0)));
  assert(cmpInst);

  /// Left branch value is somehow operand #2, right is #1.
  Value *leftBranchValue = branchInst->getOperand(2);
  Value *rightBranchValue = branchInst->getOperand(1);
  assert(leftBranchValue);
  assert(rightBranchValue);

  BasicBlock *leftBranchBB = dyn_cast<BasicBlock>(leftBranchValue);
  BasicBlock *rightBranchBB = dyn_cast<BasicBlock>(rightBranchValue);
  assert(leftBranchBB);
  assert(rightBranchBB);

  AND_OR_MutationType possibleMutationType =
    findPossibleMutationInBranch(branchInst,
                                 dyn_cast<BasicBlock>(leftBranchBB),
                                 dyn_cast<BasicBlock>(rightBranchBB));

  if (possibleMutationType == AND_OR_MutationType_AND_to_OR) {
    return applyMutationANDToOR(M, address, _V);
  }

  if (possibleMutationType == AND_OR_MutationType_OR_to_AND) {
    return applyMutationORToAND(M, address, _V);
  }

  return nullptr;
}

#pragma mark - Private: Apply mutations: AND -> OR and OR -> AND

llvm::Value *
AndOrReplacementMutationOperator::applyMutationANDToOR(Module *M,
                                                       MutationPointAddress address,
                                                       Value &_V) {

  /// In the following V argument is not used. Eventually it will be removed from
  /// this method's signature because it will be not relevant
  /// when mutations will be applied on copies of original module
  llvm::Function &F = *(std::next(M->begin(), address.getFnIndex()));
  llvm::BasicBlock &B = *(std::next(F.begin(), address.getBBIndex()));
  llvm::Instruction &I = *(std::next(B.begin(), address.getIIndex()));

  BranchInst *branchInst = dyn_cast<BranchInst>(&I);
  assert(branchInst != nullptr);
  assert(branchInst->isConditional());

  /// Operand #0 is a comparison instruction.
  CmpInst *cmpInst = (dyn_cast<CmpInst>(branchInst->getOperand(0)));
  assert(cmpInst);

  /// Left branch value is somehow operand #2, right is #1.
  Value *leftBranchValue = branchInst->getOperand(2);
  Value *rightBranchValue = branchInst->getOperand(1);
  assert(leftBranchValue);
  assert(rightBranchValue);

  BasicBlock *leftBranchBB = dyn_cast<BasicBlock>(leftBranchValue);
  BasicBlock *rightBranchBB = dyn_cast<BasicBlock>(rightBranchValue);
  assert(leftBranchBB);
  assert(rightBranchBB);

  BranchInst *leftBranchSubbranchInst = nullptr;
  for (Instruction &leftBranchInst: *leftBranchBB) {
    leftBranchSubbranchInst = dyn_cast<BranchInst>(&leftBranchInst);

    if (leftBranchSubbranchInst && leftBranchSubbranchInst->isConditional()) {
      break;
    }
  }

  assert(leftBranchSubbranchInst);

  BasicBlock *leftBranchSubbranchInst_leftBranchBB = dyn_cast<BasicBlock>(leftBranchSubbranchInst->getOperand(2));
  //BasicBlock *leftBranchSubbranchInst_rightBranchBB = dyn_cast<BasicBlock>(leftBranchSubbranchInst->getOperand(1));

  assert(leftBranchValue);

  BranchInst *replacement = BranchInst::Create(leftBranchSubbranchInst_leftBranchBB,
                                               leftBranchBB,
                                               cmpInst);

  /// If I add a named instruction, and the name already exist
  /// in a basic block, then LLVM will make another unique name of it
  /// To prevent this name change we need to 'drop' the existing old name
  branchInst->setName("");

  replacement->insertAfter(branchInst);
  branchInst->replaceAllUsesWith(replacement);
  branchInst->eraseFromParent();
  assert(branchInst->getParent() == nullptr);
  
  return replacement;
}

llvm::Value *
AndOrReplacementMutationOperator::applyMutationORToAND(Module *M,
                                                       MutationPointAddress address,
                                                       Value &_V) {

  /// In the following V argument is not used. Eventually it will be removed from
  /// this method's signature because it will be not relevant
  /// when mutations will be applied on copies of original module
  llvm::Function &F = *(std::next(M->begin(), address.getFnIndex()));
  llvm::BasicBlock &B = *(std::next(F.begin(), address.getBBIndex()));
  llvm::Instruction &I = *(std::next(B.begin(), address.getIIndex()));

  BranchInst *branchInst = dyn_cast<BranchInst>(&I);
  assert(branchInst != nullptr);
  assert(branchInst->isConditional());

  /// Operand #0 is a comparison instruction.
  CmpInst *cmpInst = (dyn_cast<CmpInst>(branchInst->getOperand(0)));
  assert(cmpInst);

  /// Left branch value is somehow operand #2, right is #1.
  Value *leftBranchValue = branchInst->getOperand(2);
  Value *rightBranchValue = branchInst->getOperand(1);
  assert(leftBranchValue);
  assert(rightBranchValue);

  BasicBlock *leftBranchBB = dyn_cast<BasicBlock>(leftBranchValue);
  BasicBlock *rightBranchBB = dyn_cast<BasicBlock>(rightBranchValue);
  assert(leftBranchBB);
  assert(rightBranchBB);

  BranchInst *rightBranchSubbranchInst = nullptr;
  for (Instruction &rightBranchInst: *rightBranchBB) {
    rightBranchSubbranchInst = dyn_cast<BranchInst>(&rightBranchInst);

    if (rightBranchSubbranchInst && rightBranchSubbranchInst->isConditional()) {
      break;
    }
  }

  assert(rightBranchSubbranchInst);

  BasicBlock *rightBranchSubbranchInst_rightBranchBB = dyn_cast<BasicBlock>(rightBranchSubbranchInst->getOperand(1));

  assert(rightBranchValue);

  BranchInst *replacement = BranchInst::Create(rightBranchBB,
                                               rightBranchSubbranchInst_rightBranchBB,
                                               cmpInst);

  /// If I add a named instruction, and the name already exist
  /// in a basic block, then LLVM will make another unique name of it
  /// To prevent this name change we need to 'drop' the existing old name
  branchInst->setName("");

  replacement->insertAfter(branchInst);
  branchInst->replaceAllUsesWith(replacement);
  branchInst->eraseFromParent();
  assert(branchInst->getParent() == nullptr);

  return replacement;
}

#pragma mark - Private: Finding possible mutations

AND_OR_MutationType
AndOrReplacementMutationOperator::findPossibleMutationInBranch(BranchInst *branchInst,
                                                               BasicBlock *leftBB,
                                                               BasicBlock *rightBB) {

  /// This filters out the cases when it is "phi" instruction that is before a
  /// branch instruction.
  if (dyn_cast<CmpInst>(branchInst->getOperand(0)) == nullptr) {
    return AND_OR_MutationType_None;
  }

  /// Operand #0 is a comparison instruction.
  assert(dyn_cast<CmpInst>(branchInst->getOperand(0)));

  /// Left branch value is somehow operand #2, right is #1.
  Value *leftBranchValue = branchInst->getOperand(2);
  assert(leftBranchValue);

  Value *rightBranchValue = branchInst->getOperand(1);
  assert(rightBranchValue);

  BasicBlock *leftBranchBB = dyn_cast<BasicBlock>(leftBranchValue);

  for (Instruction &leftBranchInst: *leftBranchBB) {
    BranchInst *leftBranchSubbranchInst = dyn_cast<BranchInst>(&leftBranchInst);

    if (leftBranchSubbranchInst == nullptr || leftBranchSubbranchInst->isConditional() == false) {
      continue;
    }

    auto leftBranchSubbranchInst_leftBB = leftBranchSubbranchInst->getOperand(2);
    auto leftBranchSubbranchInst_rightBB = leftBranchSubbranchInst->getOperand(1);

    if (leftBranchSubbranchInst_leftBB == leftBB ||
        leftBranchSubbranchInst_rightBB == leftBB) {

      return AND_OR_MutationType_OR_to_AND;

    } else if (leftBranchSubbranchInst_leftBB == rightBB ||
               leftBranchSubbranchInst_rightBB == rightBB) {

      return AND_OR_MutationType_AND_to_OR;

    } else {
      AND_OR_MutationType type =
      findPossibleMutationInBranch(leftBranchSubbranchInst, leftBB, rightBB);

      if (type != AND_OR_MutationType_None) {
        return type;
      }
    }
  }

  BasicBlock *rightBranchBB = dyn_cast<BasicBlock>(rightBranchValue);

  for (Instruction &rightBranchInst: *rightBranchBB) {
    BranchInst *rightBranchSubbranchInst = dyn_cast<BranchInst>(&rightBranchInst);

    if (rightBranchSubbranchInst == nullptr || rightBranchSubbranchInst->isConditional() == false) {
      continue;
    }

    auto rightBranchSubbranchInst_leftBB = rightBranchSubbranchInst->getOperand(2);
    auto rightBranchSubbranchInst_rightBB = rightBranchSubbranchInst->getOperand(1);

    if (rightBranchSubbranchInst_leftBB == leftBB ||
        rightBranchSubbranchInst_rightBB == leftBB) {

      return AND_OR_MutationType_OR_to_AND;

    } else if (rightBranchSubbranchInst_leftBB == rightBB ||
               rightBranchSubbranchInst_rightBB == rightBB) {

      return AND_OR_MutationType_AND_to_OR;

    } else {
      AND_OR_MutationType type =
      findPossibleMutationInBranch(rightBranchSubbranchInst, leftBB, rightBB);

      if (type != AND_OR_MutationType_None) {
        return type;
      }
    }
  }

  return AND_OR_MutationType_None;
}

#include "Mutators/AndOrReplacementMutator.h"

#include "Logger.h"
#include "MutationPoint.h"
#include "SourceLocation.h"

#include <llvm/IR/Constants.h>
#include <llvm/IR/InstIterator.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/DebugLoc.h>
#include <llvm/IR/DebugInfoMetadata.h>

#include <fstream>
#include <iterator>

using namespace llvm;
using namespace mull;

const std::string AndOrReplacementMutator::ID = "and_or_replacement_mutator";

MutationPoint *
AndOrReplacementMutator::getMutationPoint(MullModule *module,
                                          llvm::Function *function,
                                          llvm::Instruction *instruction,
                                          SourceLocation &sourceLocation,
                                          MutationPointAddress &address) {
  if (canBeApplied(*instruction)) {
    std::string diagnostics = "AND-OR Replacement";
    return new MutationPoint(this, address, instruction, function, diagnostics, sourceLocation, module);
  }
  return nullptr;
}

bool AndOrReplacementMutator::canBeApplied(Value &V) {
  BranchInst *branchInst = dyn_cast<BranchInst>(&V);

  if (branchInst == nullptr) {
    return false;
  }

  if (branchInst->isConditional() == false) {
    return false;
  }

  /// TODO: Discuss how to filter out irrelevant branch instructions.
  SourceLocation location = SourceLocation::sourceLocationFromInstruction(branchInst);
  if (!location.isNull()) {
    if (location.filePath.find("include/c++/v1") != std::string::npos) {
      return false;
    }
  }

  BranchInst *secondBranch = nullptr;
  AND_OR_MutationType possibleMutationType =
    findPossibleMutationInBranch(branchInst, &secondBranch);

  if (possibleMutationType != AND_OR_MutationType_None) {
    return true;
  }

  return false;
}

llvm::Value *AndOrReplacementMutator::applyMutation(Function *function,
                                                    MutationPointAddress &address) {
  llvm::Instruction &I = address.findInstruction(function);

  BranchInst *branchInst = dyn_cast<BranchInst>(&I);
  assert(branchInst != nullptr);
  assert(branchInst->isConditional());

  BranchInst *secondBranch = nullptr;
  AND_OR_MutationType possibleMutationType =
    findPossibleMutationInBranch(branchInst,
                                 &secondBranch);

  if (possibleMutationType == AND_OR_MutationType_AND_to_OR_Pattern1) {
    return applyMutationANDToOR_Pattern1(branchInst, secondBranch);
  }

  if (possibleMutationType == AND_OR_MutationType_AND_to_OR_Pattern2) {
    return applyMutationANDToOR_Pattern2(branchInst, secondBranch);
  }

  if (possibleMutationType == AND_OR_MutationType_AND_to_OR_Pattern3) {
    return applyMutationANDToOR_Pattern3(branchInst, secondBranch);
  }

  if (possibleMutationType == AND_OR_MutationType_OR_to_AND_Pattern1) {
    return applyMutationORToAND_Pattern1(branchInst, secondBranch);
  }

  if (possibleMutationType == AND_OR_MutationType_OR_to_AND_Pattern2) {
    return applyMutationORToAND_Pattern2(branchInst, secondBranch);
  }

  if (possibleMutationType == AND_OR_MutationType_OR_to_AND_Pattern3) {
    return applyMutationORToAND_Pattern3(branchInst, secondBranch);
  }

  return nullptr;
}

#pragma mark - Private: Apply mutations: AND -> OR

llvm::Value *
AndOrReplacementMutator::applyMutationANDToOR_Pattern1(BranchInst *firstBranch, BranchInst *secondBranch) {

  assert(firstBranch != nullptr);
  assert(firstBranch->isConditional());

  assert(secondBranch != nullptr);
  assert(secondBranch->isConditional());

  /// Operand #0 is a comparison instruction.
  Instruction *sourceInst = (dyn_cast<Instruction>(firstBranch->getOperand(0)));
  assert(sourceInst);

  /// Left branch value is somehow operand #2, right is #1.
  BasicBlock *firstBranchLeftBB  = dyn_cast<BasicBlock>(firstBranch->getOperand(2));
  BasicBlock *firstBranchRightBB = dyn_cast<BasicBlock>(firstBranch->getOperand(1));
  assert(firstBranchLeftBB);
  assert(firstBranchRightBB);

  BasicBlock *secondBranchLeftBB = dyn_cast<BasicBlock>(secondBranch->getOperand(2));

  BranchInst *replacement = BranchInst::Create(secondBranchLeftBB,
                                               firstBranchLeftBB,
                                               sourceInst);

  /// If I add a named instruction, and the name already exist
  /// in a basic block, then LLVM will make another unique name of it
  /// To prevent this name change we need to 'drop' the existing old name
  firstBranch->setName("");

  replacement->insertAfter(firstBranch);
  firstBranch->replaceAllUsesWith(replacement);

  firstBranch->eraseFromParent();

  /// If one of the second branch's successor basic blocks has a PHI node and
  /// second branch's left basic block jumps to that successor block,
  /// we need to update PHI node to also accept a jump from a replacement
  /// branch instruction.
  for (BasicBlock *secondBranchSuccessorBlock: secondBranch->getParent()->getTerminator()->successors()) {
    if (secondBranchLeftBB != secondBranchSuccessorBlock) {
      continue;
    }

    for (Instruction &inst: *secondBranchSuccessorBlock) {
      PHINode *PN = dyn_cast<PHINode>(&inst);

      if (!PN) {
        break;
      }

      int i = PN->getBasicBlockIndex(secondBranch->getParent());
      if (i < 0) {
        continue;
      }

      int operandIndex = PN->getOperandNumForIncomingValue(i);
      Value *operand = PN->getOperand(operandIndex);

      PN->addIncoming(operand, replacement->getParent());
    }
  }

  return replacement;
}

llvm::Value *
AndOrReplacementMutator::applyMutationANDToOR_Pattern2(BranchInst *firstBranch, BranchInst *secondBranch) {

  assert(firstBranch != nullptr);
  assert(firstBranch->isConditional());

  assert(secondBranch != nullptr);
  assert(secondBranch->isConditional());

  /// Operand #0 is a comparison instruction.
  Instruction *sourceInst = (dyn_cast<Instruction>(firstBranch->getOperand(0)));
  assert(sourceInst);

  /// Left branch value is somehow operand #2, right is #1.
  BasicBlock *firstBranchLeftBB  = dyn_cast<BasicBlock>(firstBranch->getOperand(2));
  BasicBlock *firstBranchRightBB = dyn_cast<BasicBlock>(firstBranch->getOperand(1));
  assert(firstBranchLeftBB);
  assert(firstBranchRightBB);

  BasicBlock *secondBranchLeftBB = dyn_cast<BasicBlock>(secondBranch->getOperand(2));

  BranchInst *replacement = BranchInst::Create(firstBranchRightBB,
                                               secondBranchLeftBB,
                                               sourceInst);

  /// If I add a named instruction, and the name already exist
  /// in a basic block, then LLVM will make another unique name of it
  /// To prevent this name change we need to 'drop' the existing old name
  firstBranch->setName("");

  replacement->insertAfter(firstBranch);
  firstBranch->replaceAllUsesWith(replacement);

  firstBranch->eraseFromParent();

  /// If one of the second branch's successor basic blocks has a PHI node and
  /// second branch's left basic block jumps to that successor block,
  /// we need to update PHI node to also accept a jump from a replacement
  /// branch instruction.
  for (BasicBlock *secondBranchSuccessorBlock: secondBranch->getParent()->getTerminator()->successors()) {
    if (secondBranchLeftBB != secondBranchSuccessorBlock) {
      continue;
    }

    for (Instruction &inst: *secondBranchSuccessorBlock) {
      PHINode *PN = dyn_cast<PHINode>(&inst);

      if (!PN) {
        break;
      }

      int i = PN->getBasicBlockIndex(secondBranch->getParent());
      if (i < 0) {
        continue;
      }

      int operandIndex = PN->getOperandNumForIncomingValue(i);
      Value *operand = PN->getOperand(operandIndex);

      PN->addIncoming(operand, replacement->getParent());
    }
  }
  
  return replacement;
}

llvm::Value *
AndOrReplacementMutator::applyMutationANDToOR_Pattern3(BranchInst *firstBranch, BranchInst *secondBranch) {
  Module *module = firstBranch->getParent()->getParent()->getParent();

  PHINode *phiNode;
  for (auto &instruction: *secondBranch->getParent()) {
    phiNode = dyn_cast<PHINode>(&instruction);

    if (phiNode == nullptr) {
      continue;
    }

    break;
  }

  assert(phiNode);

  ConstantInt *intValueForIncomingBlock0 =
  dyn_cast<ConstantInt>(phiNode->getOperand(0));
  assert(intValueForIncomingBlock0);

  bool boolValueOfIncomingBlock =
  intValueForIncomingBlock0->getValue().getBoolValue();

  ConstantInt *newValue = boolValueOfIncomingBlock ?
                            ConstantInt::getFalse(module->getContext()) :
                            ConstantInt::getTrue(module->getContext());

  phiNode->setOperand(0, newValue);

  auto firstBranchLeftBB = dyn_cast<BasicBlock>(firstBranch->getOperand(2));
  auto firstBranchRightBB = dyn_cast<BasicBlock>(firstBranch->getOperand(1));
  Instruction *firstBranchConditionInst = (dyn_cast<Instruction>(firstBranch->getOperand(0)));

  BranchInst *replacement = BranchInst::Create(firstBranchRightBB,
                                               firstBranchLeftBB,
                                               firstBranchConditionInst);

  /// If I add a named instruction, and the name already exist
  /// in a basic block, then LLVM will make another unique name of it
  /// To prevent this name change we need to 'drop' the existing old name
  firstBranch->setName("");

  replacement->insertAfter(firstBranch);
  firstBranch->replaceAllUsesWith(replacement);

  firstBranch->eraseFromParent();

  return nullptr;
}

#pragma mark - Private: Apply mutations: OR -> AND

llvm::Value *
AndOrReplacementMutator::applyMutationORToAND_Pattern1(BranchInst *firstBranch, BranchInst *secondBranch) {

  assert(firstBranch != nullptr);
  assert(firstBranch->isConditional());

  assert(secondBranch != nullptr);
  assert(secondBranch->isConditional());

  /// Operand #0 is a comparison instruction.
  Instruction *sourceInst = (dyn_cast<Instruction>(firstBranch->getOperand(0)));
  assert(sourceInst);

  /// Left branch value is somehow operand #2, right is #1.
  BasicBlock *firstBranchRightBB = dyn_cast<BasicBlock>(firstBranch->getOperand(1));
  assert(firstBranchRightBB);

  BasicBlock *secondBranchLeftBB = dyn_cast<BasicBlock>(secondBranch->getOperand(2));
  BasicBlock *secondBranchRightBB = dyn_cast<BasicBlock>(secondBranch->getOperand(1));
  assert(secondBranchLeftBB);
  assert(secondBranchRightBB);

  BranchInst *replacement = BranchInst::Create(firstBranchRightBB,
                                               secondBranchRightBB,
                                               sourceInst);

  /// If I add a named instruction, and the name already exist
  /// in a basic block, then LLVM will make another unique name of it
  /// To prevent this name change we need to 'drop' the existing old name
  firstBranch->setName("");

  replacement->insertAfter(firstBranch);
  firstBranch->replaceAllUsesWith(replacement);

  firstBranch->eraseFromParent();

  /// If one of the second branch's successor basic blocks has a PHI node and
  /// second branch's right basic block jumps to that successor block,
  /// we need to update PHI node to also accept a jump from a replacement
  /// branch instruction.
  for (BasicBlock *secondBranchSuccessorBlock: secondBranch->getParent()->getTerminator()->successors()) {
    if (secondBranchRightBB != secondBranchSuccessorBlock) {
      continue;
    }

    for (Instruction &inst: *secondBranchSuccessorBlock) {
      PHINode *PN = dyn_cast<PHINode>(&inst);

      if (!PN) {
        break;
      }

      int i = PN->getBasicBlockIndex(secondBranch->getParent());
      if (i < 0) {
        continue;
      }

      int operandIndex = PN->getOperandNumForIncomingValue(i);
      Value *operand = PN->getOperand(operandIndex);

      PN->addIncoming(operand, replacement->getParent());
    }
  }

  return replacement;
}

llvm::Value *
AndOrReplacementMutator::applyMutationORToAND_Pattern2(BranchInst *firstBranch, BranchInst *secondBranch) {

  assert(firstBranch != nullptr);
  assert(firstBranch->isConditional());

  assert(secondBranch != nullptr);
  assert(secondBranch->isConditional());

  /// Operand #0 is a comparison instruction.
  Instruction *sourceInst = (dyn_cast<Instruction>(firstBranch->getOperand(0)));
  assert(sourceInst);

  /// Left branch value is somehow operand #2, right is #1.
  BasicBlock *firstBranchLeftBB = dyn_cast<BasicBlock>(firstBranch->getOperand(2));
  assert(firstBranchLeftBB);

  BasicBlock *secondBranchLeftBB = dyn_cast<BasicBlock>(secondBranch->getOperand(2));
  BasicBlock *secondBranchRightBB = dyn_cast<BasicBlock>(secondBranch->getOperand(1));
  assert(secondBranchLeftBB);
  assert(secondBranchRightBB);

  BranchInst *replacement = BranchInst::Create(secondBranchRightBB,
                                               firstBranchLeftBB,
                                               sourceInst);

  /// If I add a named instruction, and the name already exist
  /// in a basic block, then LLVM will make another unique name of it
  /// To prevent this name change we need to 'drop' the existing old name
  firstBranch->setName("");

  replacement->insertAfter(firstBranch);
  firstBranch->replaceAllUsesWith(replacement);

  firstBranch->eraseFromParent();

  /// If one of the second branch's successor basic blocks has a PHI node and
  /// second branch's right basic block jumps to that successor block,
  /// we need to update PHI node to also accept a jump from a replacement
  /// branch instruction.
  for (BasicBlock *secondBranchSuccessorBlock: secondBranch->getParent()->getTerminator()->successors()) {
    if (secondBranchRightBB != secondBranchSuccessorBlock) {
      continue;
    }

    for (Instruction &inst: *secondBranchSuccessorBlock) {
      PHINode *PN = dyn_cast<PHINode>(&inst);

      if (!PN) {
        break;
      }

      int i = PN->getBasicBlockIndex(secondBranch->getParent());
      if (i < 0) {
        continue;
      }

      int operandIndex = PN->getOperandNumForIncomingValue(i);
      Value *operand = PN->getOperand(operandIndex);

      PN->addIncoming(operand, replacement->getParent());
    }
  }
  
  return replacement;
}

llvm::Value *
AndOrReplacementMutator::applyMutationORToAND_Pattern3(BranchInst *firstBranch, BranchInst *secondBranch) {
  Module *module = firstBranch->getParent()->getParent()->getParent();

  PHINode *phiNode;
  for (auto &instruction: *secondBranch->getParent()) {
    phiNode = dyn_cast<PHINode>(&instruction);

    if (phiNode == nullptr) {
      continue;
    }

    break;
  }
  assert(phiNode);

  ConstantInt *intValueForIncomingBlock0 =
    dyn_cast<ConstantInt>(phiNode->getOperand(0));
  assert(intValueForIncomingBlock0);

  bool boolValueOfIncomingBlock =
    intValueForIncomingBlock0->getValue().getBoolValue();

  ConstantInt *newValue = boolValueOfIncomingBlock ?
                            ConstantInt::getFalse(module->getContext()) :
                            ConstantInt::getTrue(module->getContext());

  phiNode->setOperand(0, newValue);

  auto firstBranchLeftBB = dyn_cast<BasicBlock>(firstBranch->getOperand(2));
  auto firstBranchRightBB = dyn_cast<BasicBlock>(firstBranch->getOperand(1));
  Instruction *firstBranchConditionInst = (dyn_cast<Instruction>(firstBranch->getOperand(0)));

  BranchInst *replacement = BranchInst::Create(firstBranchRightBB,
                                               firstBranchLeftBB,
                                               firstBranchConditionInst);

  /// If I add a named instruction, and the name already exist
  /// in a basic block, then LLVM will make another unique name of it
  /// To prevent this name change we need to 'drop' the existing old name
  firstBranch->setName("");

  replacement->insertAfter(firstBranch);
  firstBranch->replaceAllUsesWith(replacement);

  firstBranch->eraseFromParent();

  return nullptr;
}

#pragma mark - Private: Finding possible mutations

AND_OR_MutationType
AndOrReplacementMutator::findPossibleMutationInBranch(BranchInst *branchInst,
                                                               BranchInst **secondBranchInst) {

  if (branchInst->isConditional() == false) {
    return AND_OR_MutationType_None;
  }

  BasicBlock *leftBB = dyn_cast<BasicBlock>(branchInst->getOperand(2));
  BasicBlock *rightBB = dyn_cast<BasicBlock>(branchInst->getOperand(1));

  bool passedBranchInst = false;
  for (BasicBlock &bb: *branchInst->getFunction()) {
    for (Instruction &instruction: bb) {
      BranchInst *candidateBranchInst = dyn_cast<BranchInst>(&instruction);

      if (candidateBranchInst == nullptr ||
          candidateBranchInst->isConditional() == false) {
        continue;
      }

      if (candidateBranchInst == branchInst) {
        passedBranchInst = true;
        continue;
      }

      if (passedBranchInst == false) {
        continue;
      }

      auto candidateBranchInst_leftBB  = candidateBranchInst->getOperand(2);
      auto candidateBranchInst_rightBB = candidateBranchInst->getOperand(1);

      if (candidateBranchInst_rightBB == rightBB) {
        if (secondBranchInst) {
          *secondBranchInst = candidateBranchInst;
        }

        return AND_OR_MutationType_AND_to_OR_Pattern1;
      }

      else if (candidateBranchInst_leftBB == leftBB) {
        if (secondBranchInst) {
          *secondBranchInst = candidateBranchInst;
        }

        return AND_OR_MutationType_OR_to_AND_Pattern1;
      }

      else if (candidateBranchInst_leftBB == rightBB) {
        if (secondBranchInst) {
          *secondBranchInst = candidateBranchInst;
        }

        return AND_OR_MutationType_OR_to_AND_Pattern2;
      }

      else if (candidateBranchInst_rightBB == leftBB) {
        if (secondBranchInst) {
          *secondBranchInst = candidateBranchInst;
        }

        return AND_OR_MutationType_AND_to_OR_Pattern2;
      }

      for (auto &instruction: *candidateBranchInst->getParent()) {
        PHINode *phiNode = dyn_cast<PHINode>(&instruction);

        if (phiNode == nullptr) {
          continue;
        }

        for (BasicBlock *phiNodeIncomingBB: phiNode->blocks()) {
          if (phiNodeIncomingBB == branchInst->getParent()) {
            continue;
          }

          if (candidateBranchInst->getParent() == leftBB) {
            if (secondBranchInst) {
              *secondBranchInst = candidateBranchInst;
            }

            return AND_OR_MutationType_OR_to_AND_Pattern3;
          }

          if (candidateBranchInst->getParent() == rightBB) {
            if (secondBranchInst) {
              *secondBranchInst = candidateBranchInst;
            }

            return AND_OR_MutationType_AND_to_OR_Pattern3;
          }
        }
      }
    }
  }

  return AND_OR_MutationType_None;
}

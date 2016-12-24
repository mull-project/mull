#include "MutationOperators/AddMutationOperator.h"
#include "MutationOperators/MutationOperatorFilter.h"

#include "MutationPoint.h"
#include "Context.h"

#include "llvm/IR/InstIterator.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/DebugLoc.h"
#include "llvm/IR/DebugInfoMetadata.h"

#include <fstream>
#include <iterator>

using namespace llvm;
using namespace mull;

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
AddMutationOperator::getMutationPoints(const Context &context,
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
        MutangModule *module = context.moduleWithIdentifier(moduleID);

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

bool AddMutationOperator::canBeApplied(Value &V) {

  if (BinaryOperator *BinOp = dyn_cast<BinaryOperator>(&V)) {
    BinaryOperator::BinaryOps Opcode = BinOp->getOpcode();

    if (Opcode == Instruction::Add || Opcode == Instruction::FAdd) {
      return true;
    }

  }

  return false;
}

llvm::Value *AddMutationOperator::applyMutation(Module *M, MutationPointAddress address, Value &_V) {
  /// In the following V argument is not used. Eventually it will be removed from
  /// this method's signature because it will be not relevant
  /// when mutations will be applied on copies of original module

  llvm::Function &F = *(std::next(M->begin(), address.getFnIndex()));
  llvm::BasicBlock &B = *(std::next(F.begin(), address.getBBIndex()));
  llvm::Instruction &I = *(std::next(B.begin(), address.getIIndex()));

  /// TODO: Cover FAdd
  /// TODO: Take care of NUW/NSW
  BinaryOperator *binaryOperator = cast<BinaryOperator>(&I);

  assert(binaryOperator->getOpcode() == Instruction::Add);

  /// NOTE: Create a new BinaryOperator with the same name as existing one

  Instruction *replacement = BinaryOperator::Create(Instruction::Sub,
                                                    binaryOperator->getOperand(0),
                                                    binaryOperator->getOperand(1),
                                                    binaryOperator->getName());
  assert(replacement);
  if (binaryOperator->hasNoUnsignedWrap()) {
    replacement->setHasNoUnsignedWrap();
  }

  if (binaryOperator->hasNoSignedWrap()) {
    replacement->setHasNoSignedWrap();
  }

  /// NOTE: If I add a named instruction, and the name already exist
  /// in a basic block, then LLVM will make another unique name of it
  /// To prevent this name change we need to 'drop' the existing old name
  /// TODO: Check behaviour of 'unnamed' instructions (e.g.: %0, %2, etc.)
  binaryOperator->setName("");

  replacement->insertAfter(binaryOperator);
  binaryOperator->replaceAllUsesWith(replacement);
  binaryOperator->eraseFromParent();

  return replacement;
}

Value *AddMutationOperator::revertMutation(Value &V)  {
  LLVM_BUILTIN_UNREACHABLE;

  /*
  /// TODO: Cover FSub
  /// TODO: Take care of NUW/NSW
  BinaryOperator *BinOp = cast<BinaryOperator>(&V);

  assert(BinOp->getOpcode() == Instruction::Sub);

  /// NOTE: Create a new BinaryOperator with the same name as existing one
  Instruction *Replacement = BinaryOperator::Create(Instruction::Add,
                                                    BinOp->getOperand(0),
                                                    BinOp->getOperand(1),
                                                    BinOp->getName());

  assert(Replacement);

  /// NOTE: If I add a named instruction, and the name already exist
  /// in a basic block, then LLVM will make another unique name of it
  /// To prevent this name change we need to 'drop' the existing old name
  /// TODO: Check behaviour of 'unnamed' instructions (e.g.: %0, %2, etc.)
  BinOp->setName("");

  Replacement->insertAfter(BinOp);
  BinOp->replaceAllUsesWith(Replacement);
  BinOp->eraseFromParent();

  return Replacement;
   */
}

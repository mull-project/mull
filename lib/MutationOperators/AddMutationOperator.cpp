#include "MutationOperators/AddMutationOperator.h"

#include "MutationPoint.h"

#include "llvm/IR/InstIterator.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Module.h"

#include <iterator>

using namespace llvm;
using namespace Mutang;

bool AddMutationOperator::canBeApplied(Value &V) {

  if (BinaryOperator *BinOp = dyn_cast<BinaryOperator>(&V)) {
    BinaryOperator::BinaryOps Opcode = BinOp->getOpcode();
    return Opcode == Instruction::Add || Opcode == Instruction::FAdd;
  }

  return false;
}

llvm::Value *AddMutationOperator::applyMutation(Module *M, MutationPointAddress address, Value &V) {

  /// In the following V argument is not used. Eventually it will be removed from
  /// this method's signature because it will be not relevant
  /// when mutations will be applied on copies of original module

  llvm::Function &F = *(std::next(M->begin(), address.getFnIndex()));
  llvm::BasicBlock &B = *(std::next(F.begin(), address.getBBIndex()));
  llvm::Instruction &I = *(std::next(B.begin(), address.getIIndex()));

  /// TODO: Cover FAdd
  /// TODO: Take care of NUW/NSW
  BinaryOperator *BinOp = cast<BinaryOperator>(&I);

  assert(BinOp->getOpcode() == Instruction::Add);

  /// NOTE: Create a new BinaryOperator with the same name as existing one
  Instruction *Replacement = BinaryOperator::Create(Instruction::Sub,
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
}

Value *AddMutationOperator::revertMutation(Value &V)  {
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
}

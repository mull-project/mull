#include "MutationEngine.h"

#include "TestFinders/SimpleTestFinder.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/InstrTypes.h"
#include "llvm/IR/Module.h"

using namespace llvm;
using namespace Mutang;

/// TODO: This method should be as general as possible
void MutationEngine::applyMutation(MutationPoint &MP) {
  /// TODO: Cover FAdd
  /// TODO: Take care of NUW/NSW
  BinaryOperator *BinOp = cast<BinaryOperator>(MP.getValue());

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
}

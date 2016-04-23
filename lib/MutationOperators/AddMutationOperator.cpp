#include "MutationOperators/AddMutationOperator.h"

#include "llvm/IR/Instructions.h"

using namespace llvm;
using namespace Mutang;

bool AddMutationOperator::canBeApplied(Value &V) {

  if (BinaryOperator *BinOp = dyn_cast<BinaryOperator>(&V)) {
    BinaryOperator::BinaryOps Opcode = BinOp->getOpcode();
    return Opcode == Instruction::Add || Opcode == Instruction::FAdd;
  }

  return false;
}

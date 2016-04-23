#include "MutationOperators/AddMutationOperator.h"

#include "llvm/IR/Instructions.h"

using namespace llvm;
using namespace Mutang;

bool AddMutationOperator::canBeApplied(Value &V) {

  if (BinaryOperator *BinOp = dyn_cast<BinaryOperator>(&V)) {
    return BinOp->isBinaryOp(Instruction::Add);
  }

  return false;
}

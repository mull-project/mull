#include "MutationPoint.h"

#include "MutationOperators/MutationOperator.h"

using namespace llvm;
using namespace Mutang;

MutationPoint::MutationPoint(MutationOperator *MO, llvm::Value *Val) :
  MutOp(MO), OriginalValue(Val), MutatedValue(nullptr) {}

MutationPoint::~MutationPoint() {}

MutationOperator *MutationPoint::getOperator() {
  return MutOp;
}

Value *MutationPoint::getOriginalValue() {
  return OriginalValue;
}

Value *MutationPoint::getMutatedValue() {
  return MutatedValue;
}

void MutationPoint::applyMutation() {
  assert(!MutatedValue && "Mutation already applied");

  MutatedValue = MutOp->applyMutation(*OriginalValue);
}

void MutationPoint::revertMutation() {
  assert(MutatedValue && "Mutation was not applied yet");

  OriginalValue = MutOp->revertMutation(*MutatedValue);
}

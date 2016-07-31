#include "MutationPoint.h"

#include "MutationOperators/MutationOperator.h"

using namespace llvm;
using namespace Mutang;

MutationPoint::MutationPoint(MutationOperator *MO, MutationPointAddress Address, llvm::Value *Val) :
  MutOp(MO), Address(Address), OriginalValue(Val), MutatedValue(nullptr) {}

MutationPoint::~MutationPoint() {}

MutationOperator *MutationPoint::getOperator() {
  return MutOp;
}

MutationPointAddress MutationPoint::getAddress() {
  return Address;
}

Value *MutationPoint::getOriginalValue() {
  return OriginalValue;
}

Value *MutationPoint::getMutatedValue() {
  return MutatedValue;
}

void MutationPoint::applyMutation(llvm::Module *M) {
  assert(!MutatedValue && "Mutation already applied");

  MutatedValue = MutOp->applyMutation(M, Address, *OriginalValue);
}

void MutationPoint::revertMutation() {
  assert(MutatedValue && "Mutation was not applied yet");

  OriginalValue = MutOp->revertMutation(*MutatedValue);
}

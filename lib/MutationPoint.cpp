#include "MutationPoint.h"
#include "Compiler.h"

#include "MutationOperators/MutationOperator.h"
#include "llvm/Transforms/Utils/Cloning.h"

using namespace llvm;
using namespace Mutang;

MutationPoint::MutationPoint(MutationOperator *MO, MutationPointAddress Address, llvm::Value *Val) :
  MutOp(MO), Address(Address), OriginalValue(Val) {}

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

void MutationPoint::applyMutation(llvm::Module *M) {
  MutOp->applyMutation(M, Address, *OriginalValue);
}

object::ObjectFile *MutationPoint::applyMutation(Module *module,
                                                 Compiler &compiler) {
  if (mutatedBinary.getBinary() != nullptr) {
    return mutatedBinary.getBinary();
  }

  auto copyForMutation = CloneModule(module);
  MutOp->applyMutation(copyForMutation.get(), Address, *OriginalValue);
  mutatedBinary = compiler.CompilerModule(copyForMutation.get());

  return mutatedBinary.getBinary();
}

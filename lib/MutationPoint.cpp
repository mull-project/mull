#include "MutationPoint.h"
#include "Compiler.h"
#include "ModuleLoader.h"
#include "UniqueIDProvider.h"

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

MutationOperator *MutationPoint::getOperator() const {
  return MutOp;
}

MutationPointAddress MutationPoint::getAddress() const {
  return Address;
}

Value *MutationPoint::getOriginalValue() const {
  return OriginalValue;
}

void MutationPoint::applyMutation(llvm::Module *M) {
  MutOp->applyMutation(M, Address, *OriginalValue);
}

object::ObjectFile *MutationPoint::applyMutation(MutangModule &module,
                                                 Compiler &compiler) {
  if (mutatedBinary.getBinary() != nullptr) {
    return mutatedBinary.getBinary();
  }

  UniqueIDProvider uniqueIDProvier;
  std::string uniqueID = uniqueIDProvier.uniqueIDForModuleWithMutationPoint(module,
                                                                            *this);

  auto copyForMutation = CloneModule(module.getModule());
  MutOp->applyMutation(copyForMutation.get(), Address, *OriginalValue);
  mutatedBinary = compiler.compileModule(copyForMutation.get(), uniqueID);

  return mutatedBinary.getBinary();
}

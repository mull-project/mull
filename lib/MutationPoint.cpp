#include "MutationPoint.h"
#include "Toolchain/Compiler.h"
#include "ModuleLoader.h"

#include "MutationOperators/MutationOperator.h"
#include "llvm/Transforms/Utils/Cloning.h"

using namespace llvm;
using namespace mull;
using namespace std;

MutationPoint::MutationPoint(MutationOperator *op,
                             MutationPointAddress Address,
                             Value *Val,
                             MullModule *m) :
  mutationOperator(op), Address(Address), OriginalValue(Val), module(m)
{
  string moduleID = module->getUniqueIdentifier();
  string addressID = Address.getIdentifier();
  string operatorID = mutationOperator->uniqueID();

  uniqueIdentifier = moduleID + "_" + addressID + "_" + operatorID;
}

MutationPoint::~MutationPoint() {}

MutationOperator *MutationPoint::getOperator() {
  return mutationOperator;
}

MutationPointAddress MutationPoint::getAddress() {
  return Address;
}

Value *MutationPoint::getOriginalValue() {
  return OriginalValue;
}

MutationOperator *MutationPoint::getOperator() const {
  return mutationOperator;
}

MutationPointAddress MutationPoint::getAddress() const {
  return Address;
}

Value *MutationPoint::getOriginalValue() const {
  return OriginalValue;
}

std::unique_ptr<llvm::Module>
MutationPoint::cloneModuleAndApplyMutation() {
  auto copyForMutation = CloneModule(module->getModule());

  mutationOperator->applyMutation(copyForMutation.get(), Address, *OriginalValue);

  return copyForMutation;
}

std::string MutationPoint::getUniqueIdentifier() {
  return uniqueIdentifier;
}

std::string MutationPoint::getUniqueIdentifier() const {
  return uniqueIdentifier;
}

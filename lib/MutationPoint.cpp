#include "MutationPoint.h"
#include "Compiler.h"
#include "ModuleLoader.h"

#include "MutationOperators/MutationOperator.h"
#include "llvm/Transforms/Utils/Cloning.h"

using namespace llvm;
using namespace Mutang;
using namespace std;

MutationPoint::MutationPoint(MutationOperator *op,
                             MutationPointAddress Address,
                             Value *Val,
                             MutangModule *m) :
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

void MutationPoint::applyMutation(llvm::Module *M) {
  mutationOperator->applyMutation(M, Address, *OriginalValue);
}

llvm::object::OwningBinary<llvm::object::ObjectFile> MutationPoint::applyMutation(Compiler &compiler) {
//  if (mutatedBinary.getBinary() != nullptr) {
//    return mutatedBinary.getBinary();
//  }

  /// FIXME: Should look into cache first
  auto copyForMutation = CloneModule(module->getModule());
  printf("Compiling mutant '%s'\n", getUniqueIdentifier().c_str());
  mutationOperator->applyMutation(copyForMutation.get(), Address, *OriginalValue);
  return compiler.compileModule(copyForMutation.get());
}

std::string MutationPoint::getUniqueIdentifier() {
  return uniqueIdentifier;
}

std::string MutationPoint::getUniqueIdentifier() const {
  return uniqueIdentifier;
}

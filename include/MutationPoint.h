#pragma once

#include "llvm/ADT/ArrayRef.h"

namespace llvm {
  class Value;
  class Module;
}

namespace Mutang {

class MutationOperator;

/// \brief Container class that stores information needed to find MutationPoints.
/// We need the indexes of function, basic block and instruction to find the mutation point
/// in the clone of original module, when mutation operator is to apply mutation in that clone.
class MutationPointAddress {
  int FnIndex;
  int BBIndex;
  int IIndex;

public:
  MutationPointAddress(int FnIndex, int BBIndex, int IIndex) :
  FnIndex(FnIndex), BBIndex(BBIndex), IIndex(IIndex) {}

  int getFnIndex() { return FnIndex; }
  int getBBIndex() { return BBIndex; }
  int getIIndex() { return IIndex; }
};

class MutationPoint {
  MutationOperator *MutOp;
  MutationPointAddress Address;
  llvm::Value *OriginalValue;
  //llvm::Value *MutatedValue;
public:
  MutationPoint(MutationOperator *MO, MutationPointAddress Address, llvm::Value *Val);
  ~MutationPoint();

  MutationOperator *getOperator();

  MutationPointAddress getAddress();
  llvm::Value *getOriginalValue();
  //llvm::Value *getMutatedValue();

  void applyMutation(llvm::Module *M);
  //void revertMutation();
};

}

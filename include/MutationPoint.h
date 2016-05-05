#pragma once

#include "llvm/ADT/ArrayRef.h"

namespace llvm {
class Value;
}

namespace Mutang {

class MutationOperator;

class MutationPoint {
  MutationOperator *MutOp;
  llvm::Value *OriginalValue;
  llvm::Value *MutatedValue;
public:
  MutationPoint(MutationOperator *MO, llvm::Value *Val);
  ~MutationPoint();

  MutationOperator *getOperator();

  llvm::Value *getOriginalValue();
  llvm::Value *getMutatedValue();

  void applyMutation();
  void revertMutation();
};
  
}

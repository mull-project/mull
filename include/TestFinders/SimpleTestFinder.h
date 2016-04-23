#pragma once 

#include "llvm/ADT/ArrayRef.h"

namespace llvm {
  class Function;
  class Module;
  class Value;
}

namespace Mutang {

class Context;
class MutationOperator;

class MutationPoint {
  MutationOperator *MutOp;
  llvm::Value *Val;
public:
  MutationPoint(MutationOperator *MO, llvm::Value *V) : MutOp(MO), Val(V) {}
};

// Finds all methods that start with "test_"
class SimpleTestFinder {
  Context &Ctx;
public:
  explicit SimpleTestFinder(Context &C) : Ctx(C) {}

  llvm::ArrayRef<llvm::Function *> findTests();
  llvm::ArrayRef<llvm::Function *> findTestees(llvm::Function &F);
  llvm::ArrayRef<std::unique_ptr<MutationPoint>> findMutationPoints(
                          llvm::ArrayRef<MutationOperator *> &MutationOperators,
                          llvm::Function &F);
};

}

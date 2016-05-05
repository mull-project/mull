#pragma once 

#include "MutationPoint.h"

#include "llvm/ADT/ArrayRef.h"

namespace llvm {
  class Function;
}

namespace Mutang {

class Context;
class MutationOperator;
class MutationPoint;

// Finds all methods that start with "test_"
class SimpleTestFinder {
//  std::vector<std::unique_ptr<MutationPoint>> MutPoints;
  Context &Ctx;
public:
  explicit SimpleTestFinder(Context &C) : Ctx(C) {}

  llvm::ArrayRef<llvm::Function *> findTests();
  llvm::ArrayRef<llvm::Function *> findTestees(llvm::Function &F);
  std::vector<std::unique_ptr<MutationPoint>> findMutationPoints(
                          llvm::ArrayRef<MutationOperator *> &MutationOperators,
                          llvm::Function &F);
};

}

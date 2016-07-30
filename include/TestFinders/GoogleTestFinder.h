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

  class GoogleTestFinder {
    Context &Ctx;
  public:
    explicit GoogleTestFinder(Context &C) : Ctx(C) {}

    std::vector<llvm::Function *> findTests();
    std::vector<llvm::Function *> findTestees(llvm::Function &F);
    std::vector<std::unique_ptr<MutationPoint>> findMutationPoints(
                                                                   std::vector<MutationOperator *> &MutationOperators,
                                                                   llvm::Function &F);
  };
  
}

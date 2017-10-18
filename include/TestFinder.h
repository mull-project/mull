#pragma once

#include "MutationPoint.h"
#include "Test.h"

namespace llvm {

class Function;

}

namespace mull {

class Context;
class MutationOperator;

class TestFinder {
public:
  virtual std::vector<std::unique_ptr<Test>> findTests(Context &Ctx) = 0;

  virtual std::vector<MutationPoint *> findMutationPoints(const Context &context,
                                                          llvm::Function &F) {
    return std::vector<MutationPoint *>();
  }

  virtual ~TestFinder() {}
};

}

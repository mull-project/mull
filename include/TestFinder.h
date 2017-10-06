#pragma once

#include "MutationPoint.h"
#include "Test.h"
#include "Testee.h"

namespace llvm {

class Function;

}

namespace mull {

class Context;
class MutationOperator;

class TestFinder {
public:
  virtual std::vector<std::unique_ptr<Test>> findTests(Context &Ctx) = 0;
  virtual std::vector<std::unique_ptr<Testee>> findTestees(Test *Test,
                                                           Context &Ctx,
                                                           int maxDistance) = 0;

  virtual std::vector<IMutationPoint *> findMutationPoints(const Context &context,
                                                           llvm::Function &F) {
    return std::vector<IMutationPoint *>();
  }

  virtual ~TestFinder() {}
};

}

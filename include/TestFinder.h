#pragma once

#include "MutationPoint.h"
#include "Test.h"

namespace llvm {

class Function;

}

namespace Mutang {

class Context;
class MutationOperator;

class TestFinder {
public:
  virtual std::vector<std::unique_ptr<Test>> findTests() = 0;
  virtual std::vector<llvm::Function *> findTestees(Test *Test) = 0;
  virtual std::vector<std::unique_ptr<MutationPoint>> findMutationPoints(
                            std::vector<MutationOperator *> &MutationOperators,
                            llvm::Function &F) = 0;

  virtual ~TestFinder(){}
};

}

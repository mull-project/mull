#pragma once 

#include "TestFinder.h"
#include "Test.h"

namespace llvm {
  class Function;
}

namespace Mutang {

class Context;
class MutationOperator;
class MutationPoint;

// Finds all methods that start with "test_"
class SimpleTestFinder : public TestFinder {
  Context &Ctx;
public:
  explicit SimpleTestFinder(Context &C) : Ctx(C) {}

  std::vector<std::unique_ptr<Test>> findTests() override;
  std::vector<llvm::Function *> findTestees(Test *Test) override;
  std::vector<std::unique_ptr<MutationPoint>> findMutationPoints(
                          std::vector<MutationOperator *> &MutationOperators,
                          llvm::Function &F) override;
};

}

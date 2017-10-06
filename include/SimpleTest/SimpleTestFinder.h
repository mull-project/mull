#pragma once

#include "TestFinder.h"
#include "Test.h"

#include "MutationOperators/MutationOperator.h"

#include <map>
#include <vector>

namespace llvm {

class Function;

}

namespace mull {

class Context;
class MutationOperator;
class MutationPoint;

class SimpleTestFinder : public TestFinder {

  std::vector<std::unique_ptr<MutationPoint>> MutationPoints;
  std::vector<std::unique_ptr<MutationOperator>> mutationOperators;
  std::map<llvm::Function *, std::vector<IMutationPoint *>> MutationPointsRegistry;

public:
  SimpleTestFinder(std::vector<std::unique_ptr<MutationOperator>> mutationOperators);

  // Finds all methods that start with "test_"
  std::vector<std::unique_ptr<Test>> findTests(Context &Ctx) override;
  std::vector<std::unique_ptr<Testee>> findTestees(Test *Test,
                                                   Context &Ctx,
                                                   int distance) override;

  std::vector<IMutationPoint *> findMutationPoints(const Context &context,
                                                   llvm::Function &F) override;
};

}

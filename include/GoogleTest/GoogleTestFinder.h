#pragma once

#include "GoogleTest/GoogleTestMutationOperatorFilter.h"
#include "MutationPoint.h"
#include "TestFinder.h"

#include <llvm/ADT/StringMap.h>

#include <map>
#include <vector>

namespace llvm {

class Function;

}

namespace mull {

class Context;
class MutationOperator;
class MutationPoint;

class GoogleTestFinder : public TestFinder {
  llvm::StringMap<llvm::Function *> FunctionRegistry;
  std::vector<std::unique_ptr<MutationPoint>> MutationPoints;
  std::map<llvm::Function *, std::vector<MutationPoint *>> MutationPointsRegistry;

  std::vector<std::unique_ptr<MutationOperator>> mutationOperators;

  GoogleTestMutationOperatorFilter filter;
public:
  GoogleTestFinder(std::vector<std::unique_ptr<MutationOperator>> mutationOperators,
                   std::vector<std::string> testsToFilter,
                   std::vector<std::string> excludeLocations);

  std::vector<std::unique_ptr<Test>> findTests(Context &Ctx) override;

  std::vector<MutationPoint *> findMutationPoints(const Context &context,
                                                  llvm::Function &F) override;
};

}

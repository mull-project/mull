#pragma once

#include "MutationPoint.h"
#include "TestFinder.h"

#include "llvm/ADT/StringMap.h"

namespace llvm {

class Function;

}

namespace Mutang {

class Context;
class MutationOperator;
class MutationPoint;

class GoogleTestFinder : public TestFinder {
  llvm::StringMap<llvm::Function *> FunctionRegistry;
public:
  explicit GoogleTestFinder() {}

  std::vector<std::unique_ptr<Test>> findTests(Context &Ctx) override;
  std::vector<llvm::Function *> findTestees(Test *Test, Context &Ctx) override;
  std::vector<std::unique_ptr<MutationPoint>> findMutationPoints(
                          std::vector<MutationOperator *> &MutationOperators,
                          llvm::Function &F) override;
};

}

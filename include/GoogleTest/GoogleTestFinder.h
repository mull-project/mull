#pragma once

#include "GoogleTest/GoogleTestMutationOperatorFilter.h"
#include "TestFinder.h"

#include <llvm/ADT/StringMap.h>

#include <map>
#include <vector>

namespace llvm {

class Function;

}

namespace mull {

class Context;

class GoogleTestFinder : public TestFinder {
  llvm::StringMap<llvm::Function *> FunctionRegistry;

  GoogleTestMutationOperatorFilter filter;
public:
  GoogleTestFinder(std::vector<std::string> testsToFilter,
                   std::vector<std::string> excludeLocations);

  std::vector<std::unique_ptr<Test>> findTests(Context &Ctx) override;
};

}

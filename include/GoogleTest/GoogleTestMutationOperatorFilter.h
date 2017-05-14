#pragma once

#include "MutationOperators/MutationOperator.h"
#include "MutationOperators/MutationOperatorFilter.h"

#include <string>
#include <vector>

namespace llvm {
  class Instruction;
  class Function;
}

namespace mull {

class GoogleTestMutationOperatorFilter : public MutationOperatorFilter {

std::vector<std::string> testsToFilter;
std::vector<std::string> excludeLocations;

public:
  GoogleTestMutationOperatorFilter(std::vector<std::string> testsToFilter,
                                   std::vector<std::string> excludeLocations);

  bool shouldSkipTesteeFunction(llvm::Function *testee);
  bool shouldSkipTest(const std::string &testName);
  bool shouldSkipInstruction(llvm::Instruction *instruction);
};

}

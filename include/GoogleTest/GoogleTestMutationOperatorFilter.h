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

public:
  GoogleTestMutationOperatorFilter(std::vector<std::string> testsToFilter);

  bool shouldSkipDefinedFunction(llvm::Function *definedFunction);
  bool shouldSkipTest(const std::string &testName);
  bool shouldSkipInstruction(llvm::Instruction *instruction);
};

}

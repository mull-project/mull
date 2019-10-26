#pragma once

#include "mull/Instrumentation/Callbacks.h"
#include "mull/Instrumentation/DynamicCallTree.h"
#include "mull/ReachableFunction.h"

#include <vector>

namespace llvm {
class Module;
}

namespace mull {
class Driver;
class Test;
class Filter;

class Instrumentation {
public:
  Instrumentation();

  void recordFunctions(llvm::Module *originalModule);
  void insertCallbacks(llvm::Module *instrumentedModule);

  std::vector<std::unique_ptr<ReachableFunction>>
  getReachableFunctions(Test &test, Filter &filter, int distance);

  void setupInstrumentationInfo(Test &test);
  void cleanupInstrumentationInfo(Test &test);

  std::map<std::string, uint32_t> &getFunctionOffsetMapping();

  const char *instrumentationInfoVariableName();
  const char *functionIndexOffsetPrefix();

private:
  Callbacks callbacks;
  std::vector<CallTreeFunction> functions;
  std::map<std::string, uint32_t> functionOffsetMapping;
};
} // namespace mull

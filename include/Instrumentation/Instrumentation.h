#pragma once

#include "Instrumentation/Callbacks.h"
#include "Instrumentation/DynamicCallTree.h"
#include "Testee.h"

#include <vector>

namespace llvm {
  class Module;
}

namespace mull {
  class Toolchain;
  class Driver;
  class Test;
  class Filter;

  class Instrumentation {
  public:
    Instrumentation(Toolchain &toolchain);

    void insertCallbacks(llvm::Module *originalModule,
                         llvm::Module *instrumentedModule);

    std::vector<std::unique_ptr<Testee>> getTestees(Test *test, Filter &filter, int distance);

    void setupInstrumentationInfo(Test *test);
    void cleanupInstrumentationInfo(Test *test);
  private:
    Callbacks callbacks;
    std::vector<CallTreeFunction> functions;
  };
}

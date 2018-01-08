#pragma once

#include "Instrumentation/DynamicCallTree.h"
#include "Testee.h"

#include <cstdint>
#include <stack>

namespace mull {
  class Test;
  class Filter;

  class InstrumentationInfo {
  public:
    InstrumentationInfo();
    ~InstrumentationInfo();

    uint64_t *callTreeMapping();
    std::stack<uint64_t> &callstack();

    std::vector<std::unique_ptr<Testee>> getTestees(std::vector<CallTreeFunction> &functions, Test *test, Filter &filter, int distance);

    void prepare(size_t mappingSize);
    void cleanup(size_t mappingSize);
  private:
    DynamicCallTree dynamicCallTree;
    uint64_t *_callTreeMapping;
    std::stack<uint64_t> _callstack;
  };
}

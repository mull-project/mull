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
    InstrumentationInfo(std::vector<CallTreeFunction> &f);
    ~InstrumentationInfo();

    uint64_t *callTreeMapping();
    std::stack<uint64_t> &callstack();

    std::vector<std::unique_ptr<Testee>> getTestees(Test *test, Filter &filter, int distance);

    void prepare(size_t mappingSize);
    void reset();
  private:
    DynamicCallTree dynamicCallTree;
    uint64_t *_callTreeMapping;
    size_t mappingSize;
    std::stack<uint64_t> _callstack;
  };
}

#pragma once

#include "mull/FunctionUnderTest.h"
#include <vector>

namespace mull {

class FunctionFilter;
class progress_counter;

class FunctionFilterTask {
public:
  using In = std::vector<FunctionUnderTest>;
  using Out = std::vector<FunctionUnderTest>;
  using iterator = In::iterator;

  explicit FunctionFilterTask(FunctionFilter &filter);

  void operator()(iterator begin, iterator end, Out &storage,
                  progress_counter &counter);

private:
  FunctionFilter &filter;
};

} // namespace mull
#pragma once

#include "mull/FunctionUnderTest.h"
#include <vector>

namespace mull {
class progress_counter;

class InstructionSelectionTask {
public:
  using In = std::vector<FunctionUnderTest>;
  using Out = std::vector<int>;
  using iterator = In::iterator;

  void operator()(iterator begin, iterator end, Out &storage, progress_counter &counter);
};
} // namespace mull

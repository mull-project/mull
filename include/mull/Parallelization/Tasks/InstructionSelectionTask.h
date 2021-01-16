#pragma once

#include "mull/FunctionUnderTest.h"
#include <vector>

namespace mull {
class progress_counter;
class InstructionFilter;

class InstructionSelectionTask {
public:
  using In = std::vector<FunctionUnderTest>;
  using Out = std::vector<int>;
  using iterator = In::iterator;

  explicit InstructionSelectionTask(
      const std::vector<InstructionFilter *> &filters);
  void operator()(iterator begin, iterator end, Out &storage,
                  progress_counter &counter);

private:
  const std::vector<InstructionFilter *> &filters;
};
} // namespace mull

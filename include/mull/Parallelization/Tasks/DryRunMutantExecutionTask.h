#pragma once

#include <mull/MutationResult.h>

namespace mull {
class progress_counter;
class MutationPoint;

class DryRunMutantExecutionTask {
public:
  using In = const std::vector<MutationPoint *>;
  using Out = std::vector<std::unique_ptr<MutationResult>>;
  using iterator = In::const_iterator;

  void operator()(iterator begin, iterator end, Out &storage,
                  progress_counter &counter);
};
} // namespace mull

#pragma once

#include "mull/Core/Mutant.h"
#include "mull/Core/MutationResult.h"

namespace mull {
class progress_counter;

class DryRunMutantExecutionTask {
public:
  using In = const std::vector<std::unique_ptr<Mutant>>;
  using Out = std::vector<std::unique_ptr<MutationResult>>;
  using iterator = In::const_iterator;

  void operator()(iterator begin, iterator end, Out &storage, progress_counter &counter);
};
} // namespace mull

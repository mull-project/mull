#pragma once

#include "mull/Bitcode.h"

#include <vector>

namespace mull {
class progress_counter;
class MutationPoint;

class ApplyMutationTask {
public:
  using In = const std::vector<MutationPoint *>;
  using Out = std::vector<int>;
  using iterator = In::const_iterator;

  ApplyMutationTask() = default;

  void operator()(iterator begin, iterator end, Out &storage,
                  progress_counter &counter);
};
} // namespace mull

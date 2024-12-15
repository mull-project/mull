#pragma once

#include "mull/Bitcode.h"

#include <vector>

namespace mull {
class progress_counter;
class MutationPoint;
struct Configuration;
class Diagnostics;

class ApplyMutationTask {
public:
  explicit ApplyMutationTask(const Configuration &config, Diagnostics &diagnostics);
  using In = const std::vector<MutationPoint *>;
  using Out = std::vector<int>;
  using iterator = In::const_iterator;

  void operator()(iterator begin, iterator end, Out &storage, progress_counter &counter);

private:
  const Configuration &config;
  Diagnostics &diagnostics;
};
} // namespace mull

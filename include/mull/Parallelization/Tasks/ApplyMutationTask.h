#pragma once

#include "mull/Bitcode.h"

#include <vector>

struct MullConfig;
struct MullDiagnostics;

namespace mull {
class progress_counter;
class MutationPoint;

class ApplyMutationTask {
public:
  explicit ApplyMutationTask(const MullConfig &config, const MullDiagnostics &diagnostics);
  using In = const std::vector<MutationPoint *>;
  using Out = std::vector<int>;
  using iterator = In::const_iterator;

  void operator()(iterator begin, iterator end, Out &storage, progress_counter &counter);

private:
  const MullConfig &config;
  const MullDiagnostics &diagnostics;
};
} // namespace mull

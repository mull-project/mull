#pragma once

#include "mull/MutationResult.h"

namespace mull {

class MutationPoint;
class progress_counter;
class Diagnostics;

struct Configuration;

class MutantExecutionTask {
public:
  using In = const std::vector<MutationPoint *>;
  using Out = std::vector<std::unique_ptr<MutationResult>>;
  using iterator = In::const_iterator;

  MutantExecutionTask(const Configuration &configuration, Diagnostics &diagnostics,
                      const std::string &executable, ExecutionResult &baseline);

  void operator()(iterator begin, iterator end, Out &storage, progress_counter &counter);

private:
  const Configuration &configuration;
  Diagnostics &diagnostics;
  const std::string &executable;
  ExecutionResult &baseline;
};
} // namespace mull

#pragma once

#include "mull/Mutant.h"
#include "mull/MutationResult.h"

namespace mull {

class progress_counter;
class Diagnostics;
struct Configuration;

class MutantExecutionTask {
public:
  using In = const std::vector<std::unique_ptr<Mutant>>;
  using Out = std::vector<std::unique_ptr<MutationResult>>;
  using iterator = In::const_iterator;

  MutantExecutionTask(const Configuration &configuration, Diagnostics &diagnostics,
                      const std::string &executable, ExecutionResult &baseline,
                      const std::vector<std::string> &extraArgs);

  void operator()(iterator begin, iterator end, Out &storage, progress_counter &counter);

private:
  const Configuration &configuration;
  Diagnostics &diagnostics;
  const std::string &executable;
  ExecutionResult &baseline;
  const std::vector<std::string> &extraArgs;
};
} // namespace mull

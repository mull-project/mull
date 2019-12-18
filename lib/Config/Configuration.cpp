
#include "mull/Config/Configuration.h"

namespace mull {

ParallelizationConfig singleThreadParallelization() {
  ParallelizationConfig config;

  config.workers = 1;
  config.mutantExecutionWorkers = 1;
  config.testExecutionWorkers = 1;

  return config;
}

int MullDefaultTimeoutMilliseconds = 3000;

Configuration::Configuration()
    : dryRunEnabled(false), failFastEnabled(false), cacheEnabled(false),
      timeout(MullDefaultTimeoutMilliseconds), maxDistance(128),
      diagnostics(IDEDiagnosticsKind::None), parallelization(singleThreadParallelization()) {}

} // namespace mull

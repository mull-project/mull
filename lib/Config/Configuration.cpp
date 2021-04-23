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
unsigned MullDefaultLinkerTimeoutMilliseconds = 30000;

Configuration::Configuration()
    : debugEnabled(false), dryRunEnabled(false), captureTestOutput(true), captureMutantOutput(true),
      skipSanityCheckRun(false), includeNotCovered(false), keepObjectFiles(false),
      keepExecutable(false), mutateOnly(false), timeout(MullDefaultTimeoutMilliseconds),
      linkerTimeout(MullDefaultLinkerTimeoutMilliseconds), diagnostics(IDEDiagnosticsKind::None),
      parallelization(singleThreadParallelization()) {}

} // namespace mull

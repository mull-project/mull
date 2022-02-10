#include "mull/Config/Configuration.h"

namespace mull {

int MullDefaultTimeoutMilliseconds = 3000;
unsigned MullDefaultLinkerTimeoutMilliseconds = 30000;

Configuration::Configuration()
    : debugEnabled(false), quiet(true), silent(false), dryRunEnabled(false),
      captureTestOutput(true), captureMutantOutput(true), skipSanityCheckRun(false),
      includeNotCovered(false), keepObjectFiles(false), keepExecutable(false), mutateOnly(false),
      lowerBitcode(false), junkDetectionDisabled(false), timeout(MullDefaultTimeoutMilliseconds),
      linkerTimeout(MullDefaultLinkerTimeoutMilliseconds), diagnostics(IDEDiagnosticsKind::None),
      parallelization(ParallelizationConfig::defaultConfig()) {}

} // namespace mull

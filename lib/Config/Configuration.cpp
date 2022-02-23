#include "mull/Config/Configuration.h"

namespace mull {

int MullDefaultTimeoutMilliseconds = 3000;

Configuration::Configuration()
    : debugEnabled(false), quiet(true), silent(false), dryRunEnabled(false),
      captureTestOutput(true), captureMutantOutput(true), includeNotCovered(false),
      junkDetectionDisabled(false), timeout(MullDefaultTimeoutMilliseconds),
      diagnostics(IDEDiagnosticsKind::None),
      parallelization(ParallelizationConfig::defaultConfig()) {}

} // namespace mull

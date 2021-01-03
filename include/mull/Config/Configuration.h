#pragma once

#include <string>
#include <vector>

#include "mull/Config/ConfigurationOptions.h"

namespace mull {

extern int MullDefaultTimeoutMilliseconds;

struct Configuration {
  bool debugEnabled;
  bool dryRunEnabled;
  bool captureTestOutput;
  bool captureMutantOutput;
  bool skipSanityCheckRun;

  int timeout;

  IDEDiagnosticsKind diagnostics;

  std::vector<std::string> bitcodePaths;

  std::string executable;
  std::string coverageInfo;

  std::string linker;
  std::vector<std::string> linkerFlags;

  ParallelizationConfig parallelization;

  Configuration();
};

} // namespace mull

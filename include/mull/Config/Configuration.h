#pragma once

#include <string>
#include <vector>

#include "mull/Config/ConfigurationOptions.h"

namespace mull {

extern int MullDefaultTimeoutMilliseconds;
extern unsigned MullDefaultLinkerTimeoutMilliseconds;

struct Configuration {
  bool debugEnabled;
  bool dryRunEnabled;
  bool captureTestOutput;
  bool captureMutantOutput;
  bool skipSanityCheckRun;
  bool includeNotCovered;
  bool keepObjectFiles;
  bool keepExecutable;
  bool mutateOnly;

  int timeout;
  unsigned linkerTimeout;

  IDEDiagnosticsKind diagnostics;

  std::vector<std::string> bitcodePaths;

  std::string executable;
  std::string outputFile;
  std::string coverageInfo;

  std::string linker;
  std::vector<std::string> linkerFlags;

  ParallelizationConfig parallelization;

  Configuration();
};

} // namespace mull

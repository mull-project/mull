#pragma once

#include <string>
#include <vector>

#include "mull/Config/ConfigurationOptions.h"

namespace mull {

extern int MullDefaultTimeoutMilliseconds;

struct Configuration {
  bool debugEnabled;
  bool dryRunEnabled;
  bool failFastEnabled;
  bool cacheEnabled;
  bool captureTestOutput;
  bool captureMutantOutput;
  bool skipSanityCheckRun;

  int timeout;
  int maxDistance;

  IDEDiagnosticsKind diagnostics;

  std::vector<std::string> bitcodePaths;
  std::vector<std::string> objectFilePaths;
  std::vector<std::string> dynamicLibraryPaths;

  std::string cacheDirectory;

  std::string executable;
  std::string coverageInfo;

  std::string linker;
  std::vector<std::string> linkerFlags;

  ParallelizationConfig parallelization;
  std::vector<CustomTestDefinition> customTests;

  Configuration();
};

} // namespace mull

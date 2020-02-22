#pragma once

#include <string>
#include <vector>

#include "mull/Config/ConfigurationOptions.h"

namespace mull {

extern int MullDefaultTimeoutMilliseconds;

struct Configuration {
  bool dryRunEnabled;
  bool failFastEnabled;
  bool cacheEnabled;

  int timeout;
  int maxDistance;

  IDEDiagnosticsKind diagnostics;

  std::vector<std::string> bitcodePaths;
  std::vector<std::string> objectFilePaths;
  std::vector<std::string> dynamicLibraryPaths;

  std::string cacheDirectory;

  ParallelizationConfig parallelization;
  std::vector<CustomTestDefinition> customTests;

  Configuration();
};

} // namespace mull

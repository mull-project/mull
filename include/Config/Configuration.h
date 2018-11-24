#pragma once

#include <vector>
#include <string>

#include "Config/ConfigurationOptions.h"

namespace mull {

class RawConfig;

struct Configuration {
  bool forkEnabled;
  bool junkDetectionEnabled;
  bool dryRunEnabled;
  bool failFastEnabled;
  bool cacheEnabled;

  int timeout;
  int maxDistance;

  Diagnostics diagnostics;

  std::vector<std::string> bitcodePaths;
  std::vector<std::string> objectFilePaths;
  std::vector<std::string> dynamicLibraryPaths;

  std::string cacheDirectory;

  ParallelizationConfig parallelization;
  std::vector<CustomTestDefinition> customTests;

  Configuration();
  explicit Configuration(RawConfig &raw);
};

}


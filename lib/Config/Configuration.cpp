
#include "Config/Configuration.h"
#include "Config/RawConfig.h"

namespace mull {

ParallelizationConfig singleThreadParallelization() {
  ParallelizationConfig config;

  config.workers = 1;
  config.mutantExecutionWorkers = 1;
  config.testExecutionWorkers = 1;

  return config;
}

Configuration::Configuration() :
  forkEnabled(true),
  junkDetectionEnabled(false),
  dryRunEnabled(false),
  failFastEnabled(false),
  cacheEnabled(false),
  timeout(MullDefaultTimeoutMilliseconds),
  maxDistance(128),
  diagnostics(Diagnostics::None),
  parallelization(singleThreadParallelization())
{
}

Configuration::Configuration(RawConfig &raw) :
  forkEnabled(raw.forkEnabled()),
  junkDetectionEnabled(raw.junkDetectionEnabled()),
  dryRunEnabled(raw.dryRunModeEnabled()),
  failFastEnabled(raw.failFastModeEnabled()),
  cacheEnabled(raw.cachingEnabled()),
  timeout(raw.getTimeout()),
  maxDistance(raw.getMaxDistance()),
  diagnostics(raw.getDiagnostics()),
  bitcodePaths(raw.getBitcodePaths()),
  parallelization(raw.parallelization()),
  objectFilePaths(raw.getObjectFilesPaths()),
  dynamicLibraryPaths(raw.getDynamicLibrariesPaths()),
  cacheDirectory(raw.getCacheDirectory()),
  customTests(raw.getCustomTests())
{

}

}

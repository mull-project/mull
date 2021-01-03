#include <utility>

#include <utility>

#include <utility>

#include "mull/Config/ConfigurationOptions.h"

#include <sys/types.h>
#include <thread>

namespace mull {

ParallelizationConfig::ParallelizationConfig()
    : workers(0), testExecutionWorkers(0), mutantExecutionWorkers(0) {}

void ParallelizationConfig::normalize() {
  int defaultWorkers = std::max(std::thread::hardware_concurrency(), uint(1));
  if (workers == 0) {
    workers = defaultWorkers;
  }
  if (testExecutionWorkers == 0) {
    testExecutionWorkers = workers;
  }

  if (mutantExecutionWorkers == 0) {
    mutantExecutionWorkers = workers;
  }
}

ParallelizationConfig ParallelizationConfig::defaultConfig() {
  ParallelizationConfig config;
  config.normalize();
  return config;
}

} // namespace mull

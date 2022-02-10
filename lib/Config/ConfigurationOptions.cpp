#include "mull/Config/ConfigurationOptions.h"

#include <thread>

namespace mull {

ParallelizationConfig::ParallelizationConfig() : workers(0), executionWorkers(0) {}

void ParallelizationConfig::normalize() {
  unsigned defaultWorkers = std::max(std::thread::hardware_concurrency(), unsigned(1));
  if (workers == 0) {
    workers = defaultWorkers;
  }

  if (executionWorkers == 0) {
    executionWorkers = workers;
  }
}

bool ParallelizationConfig::exceedsHardware() {
  return workers > std::thread::hardware_concurrency();
}

ParallelizationConfig ParallelizationConfig::defaultConfig() {
  ParallelizationConfig config;
  config.normalize();
  return config;
}

} // namespace mull

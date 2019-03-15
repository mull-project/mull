#include "Config/ConfigurationOptions.h"

#include <sys/types.h>
#include <thread>

namespace mull {

std::string diagnosticsToString(Diagnostics diagnostics) {
  switch (diagnostics) {
  case Diagnostics::None: {
    return "none";
  }
  case Diagnostics::Survived: {
    return "survived";
  }
  case Diagnostics::Killed: {
    return "killed";
  }
  case Diagnostics::All: {
    return "all";
  }
  }
}

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

CustomTestDefinition::CustomTestDefinition() = default;

CustomTestDefinition::CustomTestDefinition(
    const std::string &name, const std::string &method,
    const std::string &program, const std::vector<std::string> arguments)
    : testName(name), methodName(method), programName(program),
      callArguments(arguments) {}

} // namespace mull
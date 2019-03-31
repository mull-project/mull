#pragma once

#include <string>
#include <vector>

namespace mull {

enum class Diagnostics { None, Survived, Killed, All };

std::string diagnosticsToString(Diagnostics diagnostics);

struct ParallelizationConfig {
  int workers;
  int testExecutionWorkers;
  int mutantExecutionWorkers;
  ParallelizationConfig();
  static ParallelizationConfig defaultConfig();
  void normalize();
};

struct CustomTestDefinition {
  std::string testName;
  std::string methodName;
  std::string programName;
  std::vector<std::string> callArguments;

  CustomTestDefinition();
  CustomTestDefinition(std::string name, std::string method,
                       std::string program, std::vector<std::string> arguments);
};

} // namespace mull

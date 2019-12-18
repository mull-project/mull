#pragma once

#include <string>
#include <vector>

namespace mull {

enum class IDEDiagnosticsKind { None, Survived, Killed, All };

std::string diagnosticsToString(IDEDiagnosticsKind diagnostics);

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

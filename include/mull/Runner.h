#pragma once

#include "mull/ExecutionResult.h"
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

struct MullDiagnostics;

namespace mull {

class Runner {
public:
  explicit Runner(const MullDiagnostics &diagnostics);
  ExecutionResult runProgram(const std::string &program, const std::vector<std::string> &arguments,
                             const std::unordered_map<std::string, std::string> &environment,
                             long long int timeout, bool captureOutput, bool failSilently,
                             std::optional<std::string> optionalWorkingDirectory);

private:
  const MullDiagnostics &diagnostics;
};

} // namespace mull

#pragma once

#include "mull/ExecutionResult.h"
#include <optional>
#include <string>
#include <vector>

namespace mull {

class Diagnostics;

class Runner {
public:
  explicit Runner(Diagnostics &diagnostics);
  ExecutionResult runProgram(const std::string &program, const std::vector<std::string> &arguments,
                             const std::vector<std::string> &environment, long long int timeout,
                             bool captureOutput,
                             std::optional<std::string> optionalWorkingDirectory);

private:
  Diagnostics &diagnostics;
};

} // namespace mull

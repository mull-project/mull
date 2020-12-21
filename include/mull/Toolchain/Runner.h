#pragma once

#include "mull/ExecutionResult.h"
#include <string>
#include <vector>

namespace mull {

class Diagnostics;

class Runner {
public:
  explicit Runner(Diagnostics &diagnostics);
  ExecutionResult runProgram(const std::string &program, const std::vector<std::string> &arguments,
                             const std::vector<std::string> &environment, int timeout,
                             bool captureOutput);

private:
  Diagnostics &diagnostics;
};

} // namespace mull

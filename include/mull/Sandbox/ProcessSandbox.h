#pragma once

#include "mull/ExecutionResult.h"
#include <functional>

namespace mull {

class ProcessSandbox {
public:
  const static int MullExitCode = 0;
  const static int MullTimeoutCode = 239;

  virtual ~ProcessSandbox() = default;
  virtual ExecutionResult run(std::function<ExecutionStatus()> function,
                              long long timeoutMilliseconds) const = 0;
};

class ForkProcessSandbox : public ProcessSandbox {
public:
  ExecutionResult run(std::function<ExecutionStatus()> function,
                      long long timeoutMilliseconds) const override;
};

class ForkWatchdogSandbox : public ProcessSandbox {
public:
  ExecutionResult run(std::function<ExecutionStatus()> function,
                      long long timeoutMilliseconds) const override;
};

class NullProcessSandbox : public ProcessSandbox {
public:
  ExecutionResult run(std::function<ExecutionStatus()> function,
                      long long timeoutMilliseconds) const override;
};

} // namespace mull

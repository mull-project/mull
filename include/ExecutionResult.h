#pragma once

#include <string>

namespace mull {
  enum ExecutionStatus {
    Invalid = 0,
    Failed = 1,
    Passed = 2,
    Timedout = 3,
    Crashed = 4,
    AbnormalExit = 5,
    DryRun = 6,
    FailFast = 7
  };

  struct ExecutionResult {
    ExecutionStatus status;
    int exitStatus;
    long long runningTime;
    std::string stdoutOutput;
    std::string stderrOutput;
    ExecutionResult() : status(ExecutionStatus::Invalid), exitStatus(0), runningTime(0) {}

    std::string getStatusAsString() {
      switch (this->status) {
        case Invalid:
          return "Invalid";
        case Failed:
          return "Failed";
        case Passed:
          return "Passed";
        case Timedout:
          return "Timedout";
        case Crashed:
          return "Crashed";
        case AbnormalExit:
          return "AbnormalExit";
        case DryRun:
          return "DryRun";
        case FailFast:
          return "FailFast";
      }
    }
  };
}

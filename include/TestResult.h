#pragma once

#include "MutationPoint.h"
#include "Test.h"
#include "Testee.h"

#include <vector>

namespace llvm {

class Function;

}

namespace mull {

class Test;

enum ExecutionStatus {
  Invalid = 0,
  Failed = 1,
  Passed = 2,
  Timedout = 3,
  Crashed = 4,
  AbnormalExit = 5,
  DryRun = 6
};

struct ExecutionResult {
  ExecutionStatus status;
  int exitStatus;
  long long runningTime;
  std::string stdoutOutput;
  std::string stderrOutput;

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
    }
  }
};

class MutationResult {
  ExecutionResult Result;
  MutationPoint *MutPoint;
  int distance;

public:
  MutationResult(ExecutionResult R, mull::MutationPoint *MP, int distance);

  ExecutionResult getExecutionResult()  { return Result; }
  MutationPoint* getMutationPoint()     { return MutPoint; }
  int getMutationDistance()             { return distance; }
};

class TestResult {
  /// Result of execution of a test without any mutations applied
  ExecutionResult OriginalTestResult;
  std::unique_ptr<Test> TestPtr;
  std::vector<std::unique_ptr<MutationResult>> MutationResults;
public:
  TestResult(ExecutionResult OriginalResult, std::unique_ptr<Test> T);

  void addMutantResult(std::unique_ptr<MutationResult> Res);

  std::string getTestName();
  std::string getDisplayName();

  std::vector<std::unique_ptr<MutationResult>> &getMutationResults();
  ExecutionResult getOriginalTestResult();
};

}

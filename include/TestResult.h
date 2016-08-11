#pragma once

#include "MutationPoint.h"

#include <vector>

namespace llvm {

class Function;

}

namespace Mutang {

class Test;

enum ExecutionStatus {
  Invalid = 0,
  Failed,
  Passed
};

struct ExecutionResult {
  ExecutionStatus Status;
  long long RunningTime;
};

class MutationResult {
  ExecutionResult Result;
  std::unique_ptr<MutationPoint> MutPoint;
public:
  MutationResult(ExecutionResult R, std::unique_ptr<Mutang::MutationPoint> MP);
  ExecutionResult getExecutionResult()  { return Result; }
  MutationPoint* getMutationPoint()     { return MutPoint.get(); }
};

class TestResult {
  /// Result of execution of a test without any mutations applied
  ExecutionResult OriginalTestResult;
  llvm::Function *TestFunction;
  Test *Test;
  std::vector<std::unique_ptr<MutationResult>> MutationResults;
public:
  TestResult(ExecutionResult OriginalResult, llvm::Function *TestFunction);
  TestResult(ExecutionResult OriginalResult, class Test *T);

  void addMutantResult(std::unique_ptr<MutationResult> Res);

  llvm::Function *getTestFunction();
  std::vector<std::unique_ptr<MutationResult>> &getMutationResults();
  ExecutionResult getOriginalTestResult();
};

}

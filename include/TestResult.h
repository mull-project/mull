#pragma once

#include "MutationPoint.h"

#include <vector>

namespace llvm {

class Function;

}

namespace Mutang {

enum ExecutionResult {
  Invalid = 0,
  Failed,
  Passed
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
  llvm::Function *Test;
  std::vector<std::unique_ptr<MutationResult>> MutationResults;
public:
  TestResult(ExecutionResult OriginalResult, llvm::Function *TestFunction);

  void addMutantResult(std::unique_ptr<MutationResult> Res);

  llvm::Function *getTestFunction();
  std::vector<std::unique_ptr<MutationResult>> &getMutationResults();
  ExecutionResult getOriginalTestResult();
};

}

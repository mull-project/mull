#include <utility>

#pragma once

#include "ExecutionResult.h"
#include "MutationPoint.h"
#include "mull/TestFrameworks/Test.h"

namespace mull {

class MutationResult {
  ExecutionResult Result;
  MutationPoint *MutPoint;

public:
  MutationResult(ExecutionResult R, MutationPoint *MP) : Result(std::move(R)), MutPoint(MP) {}

  ExecutionResult &getExecutionResult() {
    return Result;
  }
  MutationPoint *getMutationPoint() {
    return MutPoint;
  }
};

} // namespace mull

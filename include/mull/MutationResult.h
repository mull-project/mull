#pragma once

#include "ExecutionResult.h"
#include "MutationPoint.h"
#include <utility>

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

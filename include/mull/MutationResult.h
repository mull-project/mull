#include <utility>

#pragma once

#include "ExecutionResult.h"
#include "MutationPoint.h"
#include "mull/TestFrameworks/Test.h"

namespace mull {

class MutationResult {
  ExecutionResult Result;
  MutationPoint *MutPoint;
  int distance;
  Test *test;

public:
  MutationResult(ExecutionResult R, MutationPoint *MP, int distance, Test *test)
      : Result(std::move(R)), MutPoint(MP), distance(distance), test(test) {}

  ExecutionResult &getExecutionResult() { return Result; }
  MutationPoint *getMutationPoint() { return MutPoint; }
  int getMutationDistance() { return distance; }
  Test *getTest() { return test; }
};

} // namespace mull

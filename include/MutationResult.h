#pragma once

#include "MutationPoint.h"
#include "TestFrameworks/Test.h"
#include "ExecutionResult.h"

namespace mull {

class MutationResult {
  ExecutionResult Result;
  MutationPoint *MutPoint;
  int distance;
  Test *test;

public:
  MutationResult(ExecutionResult R,
                 MutationPoint *MP,
                 int distance,
                 Test *test) :
  Result(R), MutPoint(MP), distance(distance), test(test) {}

  ExecutionResult &getExecutionResult() { return Result; }
  MutationPoint *getMutationPoint()     { return MutPoint; }
  int getMutationDistance()             { return distance; }
  Test *getTest()                       { return test; }
};

}

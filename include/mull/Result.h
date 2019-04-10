#pragma once

#include "MutationPoint.h"
#include "MutationResult.h"
#include "mull/TestFrameworks/Test.h"
#include <vector>

namespace mull {

class Result {
  std::vector<Test> tests;
  std::vector<std::unique_ptr<MutationResult>> mutationResults;
  std::vector<MutationPoint *> mutationPoints;

public:
  Result(std::vector<Test> tests,
         std::vector<std::unique_ptr<MutationResult>> mutationResults,
         std::vector<MutationPoint *> mutationPoints)
      : tests(std::move(tests)), mutationResults(std::move(mutationResults)),
        mutationPoints(std::move(mutationPoints)) {}

  std::vector<Test> const &getTests() const { return tests; }

  std::vector<std::unique_ptr<MutationResult>> const &
  getMutationResults() const {
    return mutationResults;
  }

  std::vector<MutationPoint *> const &getMutationPoints() const {
    return mutationPoints;
  }
};
} // namespace mull

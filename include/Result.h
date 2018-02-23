#pragma once

#include "Test.h"
#include "MutationResult.h"
#include "MutationPoint.h"
#include <vector>

namespace mull {

  class Result {
    std::vector<std::unique_ptr<Test>> tests;
    std::vector<std::unique_ptr<MutationResult>> mutationResults;
    std::vector<MutationPoint *> mutationPoints;

  public:
    Result(std::vector<std::unique_ptr<Test>> tests,
           std::vector<std::unique_ptr<MutationResult>> mutationResults,
           std::vector<MutationPoint *> mutationPoints)
    : tests(std::move(tests)),
      mutationResults(std::move(mutationResults)),
      mutationPoints(std::move(mutationPoints))
    {}

    std::vector<std::unique_ptr<Test>> const& getTests() const {
      return tests;
    }

    std::vector<std::unique_ptr<MutationResult>> const& getMutationResults() const {
      return mutationResults;
    }

    std::vector<MutationPoint *> const& getMutationPoints() const {
      return mutationPoints;
    }
  };
}

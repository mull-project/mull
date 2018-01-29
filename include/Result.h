#pragma once

#include "Test.h"
#include "MutationResult.h"
#include "MutationPoint.h"
#include <vector>

namespace mull {
  struct ResultTime {
    const long start;
    const long end;

    ResultTime(const long start, const long end) : start(start), end(end) {}
  };

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

    std::vector<std::unique_ptr<Test>> const& getTests() {
      return tests;
    }

    std::vector<std::unique_ptr<MutationResult>> const& getMutationResults() {
      return mutationResults;
    }

    std::vector<MutationPoint *> const& getMutationPoints() {
      return mutationPoints;
    }
  };
}

#pragma once

#include "TestResult.h"
#include <vector>

namespace mull {
  struct ResultTime {
    const long start;
    const long end;

    ResultTime(const long start, const long end) : start(start), end(end) {}
  };

  class Result {
    std::vector<std::unique_ptr<TestResult>> testResults;

  public:
    Result(std::vector<std::unique_ptr<TestResult>> testResults) :
      testResults(std::move(testResults)) {}

    std::vector<std::unique_ptr<TestResult>> const& getTestResults() {
      return testResults;
    }
  };
}

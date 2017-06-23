#pragma once

#include "Testee.h"
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
    std::vector<std::unique_ptr<Testee>> allTestees;

  public:
    Result(std::vector<std::unique_ptr<TestResult>> testResults,
           std::vector<std::unique_ptr<Testee>> allTestees) :
      testResults(std::move(testResults)),
      allTestees(std::move(allTestees)) {}

    std::vector<std::unique_ptr<TestResult>> const& getTestResults() {
      return testResults;
    }

    std::vector<std::unique_ptr<Testee>> const& getAllTestees() {
      return allTestees;
    }

    std::vector<std::string> calculateCallerPath(MutationResult *mutationResult);
  };
}

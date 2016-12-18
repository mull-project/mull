
#include "TestResult.h"
#include <vector>

namespace Mutang {

  class Result {
    std::vector<std::unique_ptr<TestResult>> testResults;

  public:
    Result(std::vector<std::unique_ptr<TestResult>> testResults) : testResults(std::move(testResults)) {}

    std::vector<std::unique_ptr<TestResult>> const& getTestResults() {
      return testResults;
    }
  };
}

#include "TestResult.h"
#include "Test.h"

using namespace mull;

MutationResult::MutationResult(ExecutionResult R,
                               IMutationPoint *MP,
                               Testee *testee) :
  Result(R), MutPoint(MP), testee(testee) {}

TestResult::TestResult(ExecutionResult OriginalResult,
                       std::unique_ptr<Test> T) :
  OriginalTestResult(OriginalResult), TestPtr(std::move(T)) {}

void TestResult::addMutantResult(std::unique_ptr<MutationResult> Res) {
  MutationResults.push_back(std::move(Res));
}

std::string TestResult::getTestName() {
  return TestPtr->getTestName();
}

std::string TestResult::getDisplayName() {
  return TestPtr->getTestDisplayName();
}

std::vector<std::unique_ptr<MutationResult>> &TestResult::getMutationResults() {
  return MutationResults;
}

ExecutionResult TestResult::getOriginalTestResult() {
  return OriginalTestResult;
}

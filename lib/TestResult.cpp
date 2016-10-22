#include "TestResult.h"
#include "Test.h"

using namespace Mutang;

MutationResult::MutationResult(ExecutionResult R, MutationPoint *MP, int distance) :
  Result(R), MutPoint(MP), mutationDistance(distance) {

}

TestResult::TestResult(ExecutionResult OriginalResult,
                       std::unique_ptr<Test> T) :
  OriginalTestResult(OriginalResult), TestPtr(std::move(T)) {

}

void TestResult::addMutantResult(std::unique_ptr<MutationResult> Res) {
  MutationResults.push_back(std::move(Res));
}

std::string TestResult::getTestName() {
  return TestPtr->getTestName();
}

std::vector<std::unique_ptr<MutationResult>> &TestResult::getMutationResults() {
  return MutationResults;
}

ExecutionResult TestResult::getOriginalTestResult() {
  return OriginalTestResult;
}

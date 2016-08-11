#include "TestResult.h"
#include "Test.h"

using namespace Mutang;

MutationResult::MutationResult(ExecutionResult R,
                               std::unique_ptr<MutationPoint> MP) :
  Result(R), MutPoint(std::move(MP)) {

}


TestResult::TestResult(ExecutionResult OriginalResult,
                       llvm::Function *TF) :
  OriginalTestResult(OriginalResult), TestFunction(TF) {

}

TestResult::TestResult(ExecutionResult OriginalResult,
                       class Test *T) :
  OriginalTestResult(OriginalResult), Test(T) {

}

void TestResult::addMutantResult(std::unique_ptr<MutationResult> Res) {
  MutationResults.push_back(std::move(Res));
}

llvm::Function *TestResult::getTestFunction() {
  return TestFunction;
}

std::vector<std::unique_ptr<MutationResult>> &TestResult::getMutationResults() {
  return MutationResults;
}

ExecutionResult TestResult::getOriginalTestResult() {
  return OriginalTestResult;
}

#include "mull/TestFrameworks/Test.h"

#include <mull/TestFrameworks/Test.h>
#include <utility>

using namespace mull;

Test::Test(std::string test, std::string program,
           std::string driverFunctionName, std::vector<std::string> args,
           llvm::Function *testBody)
    : testName(std::move(test)), programName(std::move(program)),
      driverFunctionName(std::move(driverFunctionName)),
      arguments(std::move(args)), testBody(testBody) {}

std::string Test::getTestName() const { return testName; }
std::string Test::getProgramName() const { return programName; }
std::string Test::getDriverFunctionName() const { return driverFunctionName; }
std::string Test::getTestDisplayName() const { return getTestName(); }
std::string Test::getUniqueIdentifier() const { return getTestName(); }

const std::vector<std::string> &Test::getArguments() const { return arguments; }
const llvm::Function *Test::getTestBody() const { return testBody; }
void Test::setExecutionResult(ExecutionResult result) {
  executionResult = std::move(result);
}
const ExecutionResult &Test::getExecutionResult() const {
  return executionResult;
}

InstrumentationInfo &Test::getInstrumentationInfo() {
  return instrumentationInfo;
}

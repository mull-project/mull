#include "mull/TestFrameworks/Test.h"

#include <utility>

using namespace mull;

Test::Test(std::string test, std::string program, std::string driverFunctionName,
           std::vector<std::string> args, std::vector<const llvm::Function *> testFunctions)
    : testName(std::move(test)), programName(std::move(program)),
      driverFunctionName(std::move(driverFunctionName)), arguments(std::move(args)),
      testFunctions(std::move(testFunctions)) {}

Test::Test(std::string test, std::string program, std::string driverFunctionName,
           std::vector<std::string> args, const llvm::Function *testBody)
    : Test(std::move(test), std::move(program), std::move(driverFunctionName), std::move(args),
           std::vector<const llvm::Function *>({ testBody })) {}

std::string Test::getTestName() const {
  return testName;
}
std::string Test::getProgramName() const {
  return programName;
}
std::string Test::getDriverFunctionName() const {
  return driverFunctionName;
}
std::string Test::getTestDisplayName() const {
  return getTestName();
}
std::string Test::getUniqueIdentifier() const {
  return getTestName();
}

const std::vector<std::string> &Test::getArguments() const {
  return arguments;
}
const std::vector<const llvm::Function *> &Test::getTestFunctions() const {
  return testFunctions;
}
void Test::setExecutionResult(ExecutionResult result) {
  executionResult = std::move(result);
}
const ExecutionResult &Test::getExecutionResult() const {
  return executionResult;
}

void Test::addTestFunction(const llvm::Function *func) {
  testFunctions.push_back(func);
}

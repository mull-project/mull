#pragma once

#include <string>
#include <vector>

#include "mull/ExecutionResult.h"
#include "mull/Instrumentation/InstrumentationInfo.h"

namespace llvm {
class Function;
}

namespace mull {

class Test {
public:
  Test(std::string test, std::string program, std::string driverFunctionName,
       std::vector<std::string> args, std::vector<const llvm::Function *> testFunctions);

  // Convenience ctor for single test function usage
  Test(std::string test, std::string program, std::string driverFunctionName,
       std::vector<std::string> args, const llvm::Function *testBody);

  std::string getTestName() const;
  std::string getProgramName() const;
  std::string getDriverFunctionName() const;
  std::string getTestDisplayName() const;
  std::string getUniqueIdentifier() const;
  const std::vector<std::string> &getArguments() const;
  const std::vector<const llvm::Function *> &getTestFunctions() const;
  void addTestFunction(const llvm::Function *func);

  void setExecutionResult(ExecutionResult result);
  const ExecutionResult &getExecutionResult() const;
  InstrumentationInfo &getInstrumentationInfo();

private:
  std::string testName;
  std::string programName;
  std::string driverFunctionName;
  std::vector<std::string> arguments;
  std::vector<const llvm::Function *> testFunctions;

  ExecutionResult executionResult;
  InstrumentationInfo instrumentationInfo;
};

} // namespace mull

#include <utility>

#include "mull/Config/Configuration.h"
#include "mull/ForkProcessSandbox.h"
#include "mull/ModuleLoader.h"
#include "mull/Program/Program.h"
#include "mull/TestFrameworks/NativeTestRunner.h"
#include "mull/TestFrameworks/Test.h"
#include "mull/Toolchain/JITEngine.h"
#include "mull/Toolchain/Mangler.h"
#include "mull/Toolchain/Toolchain.h"
#include "mull/Toolchain/Trampolines.h"
#include "FixturePaths.h"

#include "gtest/gtest.h"

#include <llvm/IR/Module.h>
#include <vector>
#include <string>

using namespace mull;
using namespace llvm;
using namespace std;

using Mangler = mull::Mangler;

using ::testing::TestWithParam;
using ::testing::Values;

const static int TestTimeout = 1000;

struct CustomTestRunnerTestParameter {
  std::vector<std::string> arguments;
  ExecutionStatus status;
  CustomTestRunnerTestParameter(std::vector<std::string> arguments,
                                ExecutionStatus status)
      : arguments(std::move(arguments)), status(status) {}

  friend std::ostream &operator<<(std::ostream &os,
                                  const CustomTestRunnerTestParameter &bar) {
    os << "test arguments: {";
    for (auto &arg : bar.arguments) {
      os << "  " << arg;
    }
    os << " }\n";
    os << "expected status: " << executionStatusAsString(bar.status) << "\n";
    return os;
  }

};

class CustomTestRunnerTest : public TestWithParam<CustomTestRunnerTestParameter> {
};

TEST_P(CustomTestRunnerTest, all) {
  auto &parameter = GetParam();

  Configuration configuration;
  configuration.bitcodePaths = {mull::fixtures::custom_test_distance_bc_path(),
                                mull::fixtures::custom_test_main_bc_path(),
                                mull::fixtures::custom_test_test_bc_path()};
  Toolchain toolchain(configuration);
  NativeTestRunner runner(toolchain.mangler());

  ModuleLoader loader;
  auto loadedModules = loader.loadModules(configuration);
  Program program({}, {}, std::move(loadedModules));

  vector<object::OwningBinary<object::ObjectFile>> ownedObjects;
  vector<object::ObjectFile *> objects;
  for (auto &m : program.modules()) {
    auto object =
        toolchain.compiler().compileModule(*m, toolchain.targetMachine());
    objects.push_back(object.getBinary());
    ownedObjects.push_back(move(object));
  }

  mull::Test test("test", "mull", "main", parameter.arguments, nullptr);
  ForkProcessSandbox sandbox;
  JITEngine jit;
  std::vector<std::string> trampolineNames;
  Trampolines trampolines(trampolineNames);
  runner.loadMutatedProgram(objects, trampolines, jit);
  ExecutionResult result = sandbox.run(
      [&]() { return runner.runTest(jit, program, test); }, TestTimeout);
  ASSERT_EQ(result.status, parameter.status);
}

static const CustomTestRunnerTestParameter parameters[] = {
    CustomTestRunnerTestParameter({}, ExecutionStatus::Failed),
    CustomTestRunnerTestParameter({"arg1", "arg2"}, ExecutionStatus::Failed),
    CustomTestRunnerTestParameter({"passing_test"}, ExecutionStatus::Passed),
    CustomTestRunnerTestParameter({"failing_test"}, ExecutionStatus::Failed),
    CustomTestRunnerTestParameter({"foobar"}, ExecutionStatus::Failed),
};

INSTANTIATE_TEST_CASE_P(CustomTestRunner, CustomTestRunnerTest,
                        testing::ValuesIn(parameters));

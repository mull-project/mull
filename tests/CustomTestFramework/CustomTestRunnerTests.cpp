#include <utility>

#include "FixturePaths.h"
#include "mull/BitcodeLoader.h"
#include "mull/Config/Configuration.h"
#include "mull/Diagnostics/Diagnostics.h"
#include "mull/Program/Program.h"
#include "mull/Sandbox/ProcessSandbox.h"
#include "mull/TestFrameworks/NativeTestRunner.h"
#include "mull/TestFrameworks/Test.h"
#include "mull/Toolchain/JITEngine.h"
#include "mull/Toolchain/Mangler.h"
#include "mull/Toolchain/Toolchain.h"
#include "mull/Toolchain/Trampolines.h"

#include "gtest/gtest.h"

#include <llvm/IR/Module.h>
#include <string>
#include <vector>

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
  CustomTestRunnerTestParameter(std::vector<std::string> arguments, ExecutionStatus status)
      : arguments(std::move(arguments)), status(status) {}

  friend std::ostream &operator<<(std::ostream &os, const CustomTestRunnerTestParameter &bar) {
    os << "test arguments: {";
    for (auto &arg : bar.arguments) {
      os << "  " << arg;
    }
    os << " }\n";
    os << "expected status: " << executionStatusAsString(bar.status) << "\n";
    return os;
  }
};

class CustomTestRunnerTest : public TestWithParam<CustomTestRunnerTestParameter> {};

TEST_P(CustomTestRunnerTest, all) {
  Diagnostics diagnostics;
  auto &parameter = GetParam();

  Configuration configuration;
  configuration.bitcodePaths = { mull::fixtures::custom_test_distance_bc_path(),
                                 mull::fixtures::custom_test_main_bc_path(),
                                 mull::fixtures::custom_test_test_bc_path() };
  Toolchain toolchain(diagnostics, configuration);
  NativeTestRunner runner(diagnostics, toolchain.mangler());

  BitcodeLoader loader;
  auto bitcode = loader.loadBitcode(configuration, diagnostics);
  Program program({}, {}, std::move(bitcode));

  vector<object::OwningBinary<object::ObjectFile>> ownedObjects;
  vector<object::ObjectFile *> objects;
  for (auto &m : program.bitcode()) {
    auto object = toolchain.compiler().compileBitcode(*m, toolchain.targetMachine());
    objects.push_back(object.getBinary());
    ownedObjects.push_back(move(object));
  }

  mull::Test test("test", "mull", "main", parameter.arguments, nullptr);
  ForkTimerSandbox sandbox;
  JITEngine jit(diagnostics);
  std::vector<std::string> trampolineNames;
  Trampolines trampolines(trampolineNames);
  runner.loadMutatedProgram(objects, trampolines, jit);
  ExecutionResult result = sandbox.run(
      diagnostics, [&]() { return runner.runTest(jit, program, test); }, TestTimeout);
  ASSERT_EQ(result.status, parameter.status);
}

static const CustomTestRunnerTestParameter parameters[] = {
  CustomTestRunnerTestParameter({}, ExecutionStatus::Failed),
  CustomTestRunnerTestParameter({ "arg1", "arg2" }, ExecutionStatus::Failed),
  CustomTestRunnerTestParameter({ "passing_test" }, ExecutionStatus::Passed),
  CustomTestRunnerTestParameter({ "failing_test" }, ExecutionStatus::Failed),
  CustomTestRunnerTestParameter({ "foobar" }, ExecutionStatus::Failed),
};

INSTANTIATE_TEST_CASE_P(CustomTestRunner, CustomTestRunnerTest, testing::ValuesIn(parameters));

#include "mull/Sandbox/ProcessSandbox.h"
#include "mull/ExecutionResult.h"
#include <mull/Diagnostics/Diagnostics.h>

#include "gtest/gtest.h"

using namespace mull;

using ::testing::TestWithParam;
using ::testing::Values;

/// The timeout should be long enough to overlive the unit test suite running
/// on a cold start (fresh start without a "warmup").
static const long long Timeout = 1100;

template <typename T> class ProcessSandboxTest : public ::testing::Test {};
TYPED_TEST_CASE_P(ProcessSandboxTest);

TYPED_TEST_P(ProcessSandboxTest, captureOutputFromChildProcess) {
  Diagnostics diagnostics;
  TypeParam sandbox;

  static const char stdoutMessage[] = "Printing to stdout from a sandboxed child\n";
  static const char stderrMessage[] = "Printing to stderr from a sandboxed child\n";

  ExecutionResult result = sandbox.run(
      diagnostics,
      [&]() {
        printf("%s", stdoutMessage);
        fprintf(stderr, "%s", stderrMessage);

        return ExecutionStatus::Passed;
      },
      Timeout);

  ASSERT_EQ(result.status, Passed);

  ASSERT_STREQ(result.stdoutOutput.c_str(), stdoutMessage);
  ASSERT_STREQ(result.stderrOutput.c_str(), stderrMessage);
}

TYPED_TEST_P(ProcessSandboxTest, statusPassedIfExitingWithZeroAndResultWasSet) {
  Diagnostics diagnostics;
  TypeParam sandbox;

  ExecutionResult result = sandbox.run(
      diagnostics, [&]() { return ExecutionStatus::Passed; }, Timeout);

  ASSERT_EQ(result.status, Passed);
}

TYPED_TEST_P(ProcessSandboxTest, statusAbnormalExit_IfExitingWithNonZero) {
  Diagnostics diagnostics;
  TypeParam sandbox;

  ExecutionResult result = sandbox.run(
      diagnostics,
      [&]() {
        exit(1);
        return ExecutionStatus::Passed;
      },
      Timeout);

  ASSERT_EQ(result.status, AbnormalExit);
}

TYPED_TEST_P(ProcessSandboxTest, statusPassedIfExitedWithZero) {
  Diagnostics diagnostics;
  TypeParam sandbox;

  ExecutionResult result = sandbox.run(
      diagnostics,
      [&]() {
        exit(0);
        return ExecutionStatus::Passed;
      },
      Timeout);

  ASSERT_EQ(result.status, Passed);
}

TYPED_TEST_P(ProcessSandboxTest, statusTimeout) {
  Diagnostics diagnostics;
  TypeParam sandbox;

  ExecutionResult result = sandbox.run(
      diagnostics,
      [&]() {
        sleep(3);
        return ExecutionStatus::Passed;
      },
      Timeout);

  ASSERT_EQ(result.status, Timedout);
}

TYPED_TEST_P(ProcessSandboxTest, statusCrashed) {
  Diagnostics diagnostics;
  TypeParam sandbox;

  ExecutionResult result = sandbox.run(
      diagnostics,
      [&]() {
        abort();
        return ExecutionStatus::Passed;
      },
      Timeout);

  ASSERT_EQ(result.status, Crashed);
}

REGISTER_TYPED_TEST_CASE_P(ProcessSandboxTest, captureOutputFromChildProcess,
                           statusPassedIfExitingWithZeroAndResultWasSet,
                           statusAbnormalExit_IfExitingWithNonZero, statusPassedIfExitedWithZero,
                           statusTimeout, statusCrashed);

typedef ::testing::Types<ForkWatchdogSandbox, ForkTimerSandbox> SandboxTypes;
INSTANTIATE_TYPED_TEST_CASE_P(Mull, ProcessSandboxTest, SandboxTypes);

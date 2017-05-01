#include "ForkProcessSandbox.h"
#include "TestResult.h"

#include "gtest/gtest.h"

using namespace mull;
using namespace llvm;

/// The timeout should be long enough to overlive the unit test suite running
/// on a cold start (fresh start without a "warmup").
static const long long Timeout = 10;

TEST(ForkProcessSandbox, CaptureOutputFromChildProcess) {

  static const char StdoutMessage[] = "Printing to stdout from a sandboxed child\n";
  static const char StderrMessage[] = "Printing to stderr from a sandboxed child\n";

  ForkProcessSandbox sandbox;

  ExecutionResult result = sandbox.run([&](ExecutionResult *SharedResult) {
    ExecutionResult R;
    R.Status = Passed;
    R.RunningTime = 1;

    printf("%s", StdoutMessage);
    fprintf(stderr, "%s", StderrMessage);

    *SharedResult = R;
  }, Timeout);

  ASSERT_EQ(result.Status, Passed);
  ASSERT_EQ(result.RunningTime, 1);

  ASSERT_EQ(strcmp(result.stdoutOutput.c_str(), StdoutMessage), 0);
  ASSERT_EQ(strcmp(result.stderrOutput.c_str(), StderrMessage), 0);
}

#pragma mark - Possible execution scenarios

TEST(ForkProcessSandbox, statusPassedIfExitingWithZeroAndResultWasSet) {
  ForkProcessSandbox sandbox;

  ExecutionResult result = sandbox.run([&](ExecutionResult *SharedResult) {
    SharedResult->Status = Passed;

    exit(0);
  }, Timeout);

  ASSERT_EQ(result.Status, Passed);
}

TEST(ForkProcessSandbox, statusFailedIfExitingWithNonZero) {
  ForkProcessSandbox sandbox;

  ExecutionResult result = sandbox.run([&](ExecutionResult *SharedResult) {
    exit(1);
  }, Timeout);

  ASSERT_EQ(result.Status, Failed);
}

TEST(ForkProcessSandbox, statusFailedIfExitingWithZeroButResultWasNotSet) {
  ForkProcessSandbox sandbox;

  ExecutionResult result = sandbox.run([&](ExecutionResult *SharedResult) {
    exit(0);
  }, Timeout);

  ASSERT_EQ(result.Status, Failed);
}


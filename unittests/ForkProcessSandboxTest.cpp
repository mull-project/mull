#include "ForkProcessSandbox.h"
#include "TestResult.h"

#include "gtest/gtest.h"

using namespace mull;
using namespace llvm;

/// The timeout should be long enough to overlive the unit test suite running
/// on a cold start (fresh start without a "warmup").
static const long long Timeout = 1000;

TEST(ForkProcessSandbox, captureOutputFromChildProcess) {

  static const char stdoutMessage[] = "Printing to stdout from a sandboxed child\n";
  static const char stderrMessage[] = "Printing to stderr from a sandboxed child\n";

  ForkProcessSandbox sandbox;

  ExecutionResult result = sandbox.run([&](ExecutionResult *SharedResult) {
    ExecutionResult R;
    R.Status = Passed;
    R.RunningTime = 1;

    printf("%s", stdoutMessage);
    fprintf(stderr, "%s", stderrMessage);

    *SharedResult = R;
  }, Timeout);

  ASSERT_EQ(result.Status, Passed);
  ASSERT_EQ(result.RunningTime, 1);

  ASSERT_EQ(strcmp(result.stdoutOutput.c_str(), stdoutMessage), 0);
  ASSERT_EQ(strcmp(result.stderrOutput.c_str(), stderrMessage), 0);
}

#pragma mark - Possible execution scenarios

TEST(ForkProcessSandbox, statusPassedIfExitingWithZeroAndResultWasSet) {
  ForkProcessSandbox sandbox;

  ExecutionResult result = sandbox.run([&](ExecutionResult *SharedResult) {
    SharedResult->Status = Passed;
  }, Timeout);

  ASSERT_EQ(result.Status, Passed);
}

TEST(ForkProcessSandbox, statusAbnormalExit_IfExitingWithNonZero) {
  ForkProcessSandbox sandbox;

  ExecutionResult result = sandbox.run([&](ExecutionResult *SharedResult) {
    exit(1);
  }, Timeout);

  ASSERT_EQ(result.Status, AbnormalExit);
}

TEST(ForkProcessSandbox, statusAbnormalExit_IfExitingWithZeroButResultWasNotSet) {
  ForkProcessSandbox sandbox;

  ExecutionResult result = sandbox.run([&](ExecutionResult *SharedResult) {
    exit(0);
  }, Timeout);

  ASSERT_EQ(result.Status, AbnormalExit);
}

TEST(ForkProcessSandbox, statusTimeout) {
  ForkProcessSandbox sandbox;

  ExecutionResult result = sandbox.run([&](ExecutionResult *_) {
    sleep(3);
  }, Timeout);

  ASSERT_EQ(result.Status, Timedout);
}

TEST(ForkProcessSandbox, statusCrashed) {
  ForkProcessSandbox sandbox;

  ExecutionResult result = sandbox.run([&](ExecutionResult *_) {
    abort();
  }, Timeout);

  ASSERT_EQ(result.Status, Crashed);
}

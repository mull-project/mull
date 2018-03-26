#include "ForkProcessSandbox.h"
#include "ExecutionResult.h"

#include "gtest/gtest.h"

using namespace mull;
//using namespace llvm;

/// The timeout should be long enough to overlive the unit test suite running
/// on a cold start (fresh start without a "warmup").
static const long long Timeout = 1100;

TEST(ForkProcessSandbox, captureOutputFromChildProcess) {

  static const char stdoutMessage[] = "Printing to stdout from a sandboxed child\n";
  static const char stderrMessage[] = "Printing to stderr from a sandboxed child\n";

  ForkProcessSandbox sandbox;

  ExecutionResult result = sandbox.run([&]() {
    printf("%s", stdoutMessage);
    fprintf(stderr, "%s", stderrMessage);

    return ExecutionStatus::Passed;
  }, Timeout);

  ASSERT_EQ(result.status, Passed);

  ASSERT_EQ(strcmp(result.stdoutOutput.c_str(), stdoutMessage), 0);
  ASSERT_EQ(strcmp(result.stderrOutput.c_str(), stderrMessage), 0);
}

#pragma mark - Possible execution scenarios

TEST(ForkProcessSandbox, statusPassedIfExitingWithZeroAndResultWasSet) {
  ForkProcessSandbox sandbox;

  ExecutionResult result = sandbox.run([&]() {
    return ExecutionStatus::Passed;
  }, Timeout);

  ASSERT_EQ(result.status, Passed);
}

TEST(ForkProcessSandbox, statusAbnormalExit_IfExitingWithNonZero) {
  ForkProcessSandbox sandbox;

  ExecutionResult result = sandbox.run([&]() {
    exit(1);
    return ExecutionStatus::Passed;
  }, Timeout);

  ASSERT_EQ(result.status, AbnormalExit);
}

TEST(ForkProcessSandbox, statusAbnormalExit_IfExitingWithZeroButResultWasNotSet) {
  ForkProcessSandbox sandbox;

  ExecutionResult result = sandbox.run([&]() {
    exit(0);
    return ExecutionStatus::Passed;
  }, Timeout);

  ASSERT_EQ(result.status, AbnormalExit);
}

TEST(ForkProcessSandbox, statusTimeout) {
  ForkProcessSandbox sandbox;

  ExecutionResult result = sandbox.run([&]() {
    sleep(3);
    return ExecutionStatus::Passed;
  }, Timeout);

  ASSERT_EQ(result.status, Timedout);
}

TEST(ForkProcessSandbox, statusCrashed) {
  ForkProcessSandbox sandbox;

  ExecutionResult result = sandbox.run([&]() {
    abort();
    return ExecutionStatus::Passed;
  }, Timeout);

  ASSERT_EQ(result.status, Crashed);
}

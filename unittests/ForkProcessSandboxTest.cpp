#include "ForkProcessSandbox.h"
#include "TestResult.h"

#include "gtest/gtest.h"

using namespace Mutang;
using namespace llvm;

TEST(ForkProcessSandbox, CaptureOutputFromChildProcess) {

  static const char StdoutMessage[] = "Printing to stdout from a sandboxed child\n";
  static const char StderrMessage[] = "Printing to stderr from a sandboxed child\n";

  static const long long IrrelevantValue = 2;

  ForkProcessSandbox sandbox;

  ExecutionResult result = sandbox.run([&](ExecutionResult *SharedResult) {
    ExecutionResult R;
    R.Status = Passed;
    R.RunningTime = 1;

    printf("%s", StdoutMessage);
    fprintf(stderr, "%s", StderrMessage);

    *SharedResult = R;
  }, IrrelevantValue);

  ASSERT_EQ(result.Status, Passed);
  ASSERT_EQ(result.RunningTime, 1);

  ASSERT_EQ(strcmp(result.stdoutOutput.c_str(), StdoutMessage), 0);
  ASSERT_EQ(strcmp(result.stderrOutput.c_str(), StderrMessage), 0);
}

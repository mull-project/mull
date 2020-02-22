#include "mull/Parallelization/Tasks/OriginalTestExecutionTask.h"

#include "mull/Config/Configuration.h"
#include "mull/Diagnostics/Diagnostics.h"
#include "mull/Instrumentation/Instrumentation.h"
#include "mull/Parallelization/Progress.h"
#include "mull/Sandbox/ProcessSandbox.h"
#include "mull/TestFrameworks/TestRunner.h"
#include "mull/Toolchain/Toolchain.h"

#include <sstream>

using namespace mull;
using namespace llvm;

OriginalTestExecutionTask::OriginalTestExecutionTask(
    Diagnostics &diagnostics, Instrumentation &instrumentation, Program &program,
    const ProcessSandbox &sandbox, TestRunner &runner, const Configuration &config, JITEngine &jit)
    : diagnostics(diagnostics), instrumentation(instrumentation), program(program),
      sandbox(sandbox), runner(runner), config(config), jit(jit) {}

void OriginalTestExecutionTask::operator()(iterator begin, iterator end, Out &storage,
                                           progress_counter &counter) {
  for (auto it = begin; it != end; ++it, counter.increment()) {
    auto &test = *it;

    instrumentation.setupInstrumentationInfo(test);

    ExecutionResult testExecutionResult = sandbox.run(
        diagnostics, [&]() { return runner.runTest(jit, program, test); }, config.timeout);

    test.setExecutionResult(testExecutionResult);

    std::vector<std::unique_ptr<ReachableFunction>> reachableFunctions;

    if (testExecutionResult.status == Passed) {
      reachableFunctions = instrumentation.getReachableFunctions(test, config.maxDistance);
    } else {
      std::stringstream failureMessage;
      failureMessage << "Original test failed\n";
      failureMessage << "test: ";
      failureMessage << test.getTestName() << "\n";
      failureMessage << "status: ";
      failureMessage << testExecutionResult.getStatusAsString() << "\n";
      failureMessage << "stdout: '";
      failureMessage << testExecutionResult.stdoutOutput << "'\n";
      failureMessage << "stderr: '";
      failureMessage << testExecutionResult.stderrOutput << "'\n";
      diagnostics.warning(failureMessage.str());
    }
    instrumentation.cleanupInstrumentationInfo(test);

    if (reachableFunctions.empty()) {
      continue;
    }

    for (auto it = std::next(reachableFunctions.begin()); it != reachableFunctions.end(); ++it) {
      storage.push_back(std::move(*it));
    }
  }
}

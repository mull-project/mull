#include "mull/Parallelization/Tasks/OriginalTestExecutionTask.h"

#include "mull/Config/Configuration.h"
#include "mull/Instrumentation/Instrumentation.h"
#include "mull/Logger.h"
#include "mull/Parallelization/Progress.h"
#include "mull/Sandbox/ProcessSandbox.h"
#include "mull/TestFrameworks/TestRunner.h"
#include "mull/Toolchain/Toolchain.h"

#include <sstream>

using namespace mull;
using namespace llvm;

OriginalTestExecutionTask::OriginalTestExecutionTask(
    Instrumentation &instrumentation, Program &program,
    const ProcessSandbox &sandbox, TestRunner &runner,
    const Configuration &config, Filter &filter, JITEngine &jit)
    : instrumentation(instrumentation), program(program), sandbox(sandbox),
      runner(runner), config(config), filter(filter), jit(jit) {}

void OriginalTestExecutionTask::operator()(iterator begin, iterator end,
                                           Out &storage,
                                           progress_counter &counter) {
  for (auto it = begin; it != end; ++it, counter.increment()) {
    auto &test = *it;

    instrumentation.setupInstrumentationInfo(test);

    ExecutionResult testExecutionResult = sandbox.run(
        [&]() { return runner.runTest(jit, program, test); }, config.timeout);

    test.setExecutionResult(testExecutionResult);

    std::vector<std::unique_ptr<Testee>> testees;

    if (testExecutionResult.status == Passed) {
      testees = instrumentation.getTestees(test, filter, config.maxDistance);
    } else {
      std::stringstream failureMessage;
      failureMessage << "\n";
      failureMessage << "test: ";
      failureMessage << test.getTestName() << "\n";
      failureMessage << "status: ";
      failureMessage << testExecutionResult.getStatusAsString() << "\n";
      failureMessage << "stdout: '";
      failureMessage << testExecutionResult.stdoutOutput << "'\n";
      failureMessage << "stderr: '";
      failureMessage << testExecutionResult.stderrOutput << "'\n";
      Logger::error() << failureMessage.str();
    }
    instrumentation.cleanupInstrumentationInfo(test);

    if (testees.empty()) {
      continue;
    }

    for (auto it = std::next(testees.begin()); it != testees.end(); ++it) {
      storage.push_back(std::move(*it));
    }
  }
}

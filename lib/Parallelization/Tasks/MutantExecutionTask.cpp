#include "mull/Parallelization/Tasks/MutantExecutionTask.h"

#include "mull/Config/Configuration.h"
#include "mull/Parallelization/Progress.h"
#include "mull/Sandbox/ProcessSandbox.h"
#include "mull/TestFrameworks/TestRunner.h"

#include <llvm/ExecutionEngine/ExecutionEngine.h>
#include <llvm/Support/TargetSelect.h>

using namespace mull;
using namespace llvm;

MutantExecutionTask::MutantExecutionTask(Diagnostics &diagnostics, const ProcessSandbox &sandbox,
                                         Program &program, TestRunner &runner,
                                         const Configuration &config,
                                         std::vector<llvm::object::ObjectFile *> &objectFiles)
    : diagnostics(diagnostics), jit(diagnostics), program(program), sandbox(sandbox),
      runner(runner), config(config), objectFiles(objectFiles) {}

void MutantExecutionTask::operator()(iterator begin, iterator end, Out &storage,
                                     progress_counter &counter) {
  runner.loadMutatedProgram(objectFiles, jit);

  for (auto it = begin; it != end; ++it, counter.increment()) {
    auto mutationPoint = *it;

    auto atLeastOneTestFailed = false;
    for (auto &reachableTest : mutationPoint->getReachableTests()) {
      auto test = reachableTest.first;
      auto distance = reachableTest.second;

      ExecutionResult result;
      if (config.failFastEnabled && atLeastOneTestFailed) {
        result.status = ExecutionStatus::FailFast;
      } else {
        const auto timeout = test->getExecutionResult().runningTime * 10;
        const auto sandboxTimeout = std::max(30LL, timeout);

        result = sandbox.run(
            diagnostics,
            [&]() {
              setenv(mutationPoint->getUserIdentifier().c_str(), "1", 1);
              ExecutionStatus status = runner.runTest(jit, program, *test);
              unsetenv(mutationPoint->getUserIdentifier().c_str());
              assert(status != ExecutionStatus::Invalid && "Expect to see valid TestResult");
              return status;
            },
            sandboxTimeout,
            config.captureMutantOutput);

        assert(result.status != ExecutionStatus::Invalid && "Expect to see valid TestResult");

        if (result.status != ExecutionStatus::Passed) {
          atLeastOneTestFailed = true;
        }
      }

      storage.push_back(std::make_unique<MutationResult>(result, mutationPoint, distance, test));
    }
  }
}

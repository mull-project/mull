#include "mull/Parallelization/Tasks/MutantExecutionTask.h"

#include "mull/Config/Configuration.h"
#include "mull/Parallelization/Progress.h"
#include "mull/Sandbox/ProcessSandbox.h"
#include "mull/TestFrameworks/TestRunner.h"
#include "mull/Toolchain/Mangler.h"
#include "mull/Toolchain/Trampolines.h"

#include <llvm/ExecutionEngine/ExecutionEngine.h>
#include <llvm/Support/TargetSelect.h>

using namespace mull;
using namespace llvm;

MutantExecutionTask::MutantExecutionTask(Diagnostics &diagnostics, const ProcessSandbox &sandbox,
                                         Program &program, TestRunner &runner,
                                         const Configuration &config, Mangler &mangler,
                                         std::vector<llvm::object::ObjectFile *> &objectFiles,
                                         std::vector<std::string> &mutatedFunctionNames)
    : diagnostics(diagnostics), jit(diagnostics), sandbox(sandbox), program(program),
      runner(runner), config(config), mangler(mangler), objectFiles(objectFiles),
      mutatedFunctionNames(mutatedFunctionNames) {}

void MutantExecutionTask::operator()(iterator begin, iterator end, Out &storage,
                                     progress_counter &counter) {
  Trampolines trampolines(mutatedFunctionNames);
  runner.loadMutatedProgram(objectFiles, trampolines, jit);
  trampolines.fixupOriginalFunctions(jit);

  for (auto it = begin; it != end; ++it, counter.increment()) {
    auto mutationPoint = *it;

    auto trampolineName = mangler.getNameWithPrefix(mutationPoint->getTrampolineName());
    auto mutatedFunctionName = mangler.getNameWithPrefix(mutationPoint->getMutatedFunctionName());
    uint64_t *trampoline = trampolines.findTrampoline(trampolineName);
    uint64_t address = llvm_compat::JITSymbolAddress(jit.getSymbol(mutatedFunctionName));
    uint64_t originalAddress = *trampoline;
    *trampoline = address;

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
              ExecutionStatus status = runner.runTest(jit, program, *test);
              assert(status != ExecutionStatus::Invalid && "Expect to see valid TestResult");
              return status;
            },
            sandboxTimeout);

        assert(result.status != ExecutionStatus::Invalid && "Expect to see valid TestResult");

        if (result.status != ExecutionStatus::Passed) {
          atLeastOneTestFailed = true;
        }
      }

      storage.push_back(make_unique<MutationResult>(result, mutationPoint, distance, test));
    }

    *trampoline = originalAddress;
  }
}

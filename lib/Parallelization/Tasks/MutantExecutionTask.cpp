#include "Parallelization/Tasks/MutantExecutionTask.h"
#include "Parallelization/Progress.h"
#include "Driver.h"
#include "Config.h"
#include "TestRunner.h"

#include <llvm/ExecutionEngine/ExecutionEngine.h>
#include <llvm/Support/TargetSelect.h>

using namespace mull;
using namespace llvm;

mull::MutantExecutionTask::MutantExecutionTask(Driver &driver,
                                               ProcessSandbox &sandbox,
                                               TestRunner &runner,
                                               Config &config,
                                               Toolchain &toolchain,
                                               Filter &filter)
    : sandbox(sandbox), runner(runner),
      config(config), toolchain(toolchain), filter(filter), driver(driver) {}

void MutantExecutionTask::operator()(MutantExecutionTask::iterator begin,
                                     MutantExecutionTask::iterator end,
                                     MutantExecutionTask::Out &storage,
                                     progress_counter &counter) {
  EngineBuilder builder;
  auto target = builder.selectTarget(llvm::Triple(), "", "",
                                     llvm::SmallVector<std::string, 1>());
  std::unique_ptr<TargetMachine> localMachine(target);

  for (auto it = begin; it != end; ++it, counter.increment()) {
    auto mutationPoint = *it;
    auto objectFilesWithMutant = driver.AllButOne(mutationPoint->getOriginalModule()->getModule());

    auto mutant = toolchain.cache().getObject(*mutationPoint);
    if (mutant.getBinary() == nullptr) {
      LLVMContext localContext;
      auto clonedModule = mutationPoint->getOriginalModule()->clone(localContext);
      mutationPoint->applyMutation(*clonedModule.get());
      mutant = toolchain.compiler().compileModule(*clonedModule.get(), *localMachine);
      toolchain.cache().putObject(mutant, *mutationPoint);
    }

    objectFilesWithMutant.push_back(mutant.getBinary());

    runner.loadProgram(objectFilesWithMutant, jit);

    auto atLeastOneTestFailed = false;
    for (auto &reachableTest : mutationPoint->getReachableTests()) {
      auto test = reachableTest.first;
      auto distance = reachableTest.second;

      ExecutionResult result;
      if (config.failFastModeEnabled() && atLeastOneTestFailed) {
        result.status = ExecutionStatus::FailFast;
      } else {
        const auto timeout = test->getExecutionResult().runningTime * 10;
        const auto sandboxTimeout = std::max(30LL, timeout);

        result = sandbox.run([&]() {
          ExecutionStatus status = runner.runTest(test, jit);
          assert(status != ExecutionStatus::Invalid && "Expect to see valid TestResult");
          return status;
        }, sandboxTimeout);

        assert(result.status != ExecutionStatus::Invalid &&
            "Expect to see valid TestResult");

        if (result.status != ExecutionStatus::Passed) {
          atLeastOneTestFailed = true;
        }
      }

      storage.push_back(make_unique<MutationResult>(result, mutationPoint, distance, test));
    }
  }
}

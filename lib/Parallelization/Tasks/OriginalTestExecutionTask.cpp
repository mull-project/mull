#include "Parallelization/Tasks/OriginalTestExecutionTask.h"
#include "Parallelization/Progress.h"
#include "Instrumentation/Instrumentation.h"
#include "Toolchain/Toolchain.h"
#include "ForkProcessSandbox.h"
#include "TestRunner.h"
#include "Config/RawConfig.h"

using namespace mull;
using namespace llvm;

OriginalTestExecutionTask::OriginalTestExecutionTask(Instrumentation &instrumentation,
                                                     ProcessSandbox &sandbox,
                                                     TestRunner &runner,
                                                     RawConfig &config,
                                                     Filter &filter,
                                                     JITEngine &jit)
    : instrumentation(instrumentation),
      sandbox(sandbox),
      runner(runner),
      config(config),
      filter(filter),
      jit(jit) {}

void OriginalTestExecutionTask::operator()(iterator begin, iterator end, Out &storage,
                                           progress_counter &counter) {
  for (auto it = begin; it != end; ++it, counter.increment()) {
    auto &test = *it;

    instrumentation.setupInstrumentationInfo(test.get());

    ExecutionResult testExecutionResult = sandbox.run([&]() {
      return runner.runTest(test.get(), jit);
    }, config.getTimeout());

    test->setExecutionResult(testExecutionResult);

    std::vector<std::unique_ptr<Testee>> testees;

    if (testExecutionResult.status == Passed) {
      testees = instrumentation.getTestees(test.get(), filter, config.getMaxDistance());
    } else {
      auto ssss = test->getTestName() + " failed: " + testExecutionResult.getStatusAsString() + "\n";
      errs() << ssss;
    }
    instrumentation.cleanupInstrumentationInfo(test.get());

    if (testees.empty()) {
      continue;
    }

    for (auto it = std::next(testees.begin()); it != testees.end(); ++it) {
      storage.push_back(std::move(*it));
    }
  }
}

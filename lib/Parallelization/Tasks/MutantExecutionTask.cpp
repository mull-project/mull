#include "mull/Parallelization/Tasks/MutantExecutionTask.h"

#include "mull/Config/Configuration.h"
#include "mull/Diagnostics/Diagnostics.h"
#include "mull/Parallelization/Progress.h"
#include "mull/Toolchain/Runner.h"

using namespace mull;
using namespace std::string_literals;

MutantExecutionTask::MutantExecutionTask(const Configuration &configuration,
                                         Diagnostics &diagnostics, const std::string &executable,
                                         ExecutionResult &baseline)
    : configuration(configuration), diagnostics(diagnostics), executable(executable),
      baseline(baseline) {}

void MutantExecutionTask::operator()(iterator begin, iterator end, Out &storage,
                                     progress_counter &counter) {
  Runner runner(diagnostics);
  for (auto it = begin; it != end; ++it, counter.increment()) {
    MutationPoint *mutationPoint = *it;
    ExecutionResult result = runner.runProgram(executable,
                                               {},
                                               { mutationPoint->getUserIdentifier() },
                                               baseline.runningTime * 10,
                                               configuration.captureMutantOutput);

    storage.push_back(std::make_unique<MutationResult>(result, mutationPoint));
  }
}

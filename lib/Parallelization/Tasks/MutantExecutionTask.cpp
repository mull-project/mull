#include "mull/Parallelization/Tasks/MutantExecutionTask.h"

#include "mull/Config/Configuration.h"
#include "mull/Diagnostics/Diagnostics.h"
#include "mull/Parallelization/Progress.h"
#include "mull/Toolchain/Runner.h"

using namespace mull;
using namespace std::string_literals;

MutantExecutionTask::MutantExecutionTask(const Configuration &configuration,
                                         Diagnostics &diagnostics, const std::string &executable)
    : configuration(configuration), diagnostics(diagnostics), executable(executable) {}

void MutantExecutionTask::operator()(iterator begin, iterator end, Out &storage,
                                     progress_counter &counter) {
  Runner runner(diagnostics);
  ExecutionResult baselineResult = runner.runProgram(
      executable, {}, {}, configuration.timeout, configuration.captureMutantOutput);
  diagnostics.debug("baseline "s + std::to_string(baselineResult.runningTime));
  for (auto it = begin; it != end; ++it, counter.increment()) {
    MutationPoint *mutationPoint = *it;
    Test *test = mutationPoint->getReachableTests().front().first;
    ExecutionResult result = runner.runProgram(executable,
                                               {},
                                               { mutationPoint->getUserIdentifier() },
                                               baselineResult.runningTime * 10,
                                               configuration.captureMutantOutput);

    storage.push_back(std::make_unique<MutationResult>(result, mutationPoint, 1, test));
  }
}

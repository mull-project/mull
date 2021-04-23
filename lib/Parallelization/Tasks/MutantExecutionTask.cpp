#include "mull/Parallelization/Tasks/MutantExecutionTask.h"

#include "mull/Config/Configuration.h"
#include "mull/Diagnostics/Diagnostics.h"
#include "mull/Parallelization/Progress.h"
#include "mull/Toolchain/Runner.h"

using namespace mull;
using namespace std::string_literals;

MutantExecutionTask::MutantExecutionTask(const Configuration &configuration,
                                         Diagnostics &diagnostics, const std::string &executable,
                                         ExecutionResult &baseline,
                                         const std::vector<std::string> &extraArgs)
    : configuration(configuration), diagnostics(diagnostics), executable(executable),
      baseline(baseline), extraArgs(extraArgs) {}

void MutantExecutionTask::operator()(iterator begin, iterator end, Out &storage,
                                     progress_counter &counter) {
  Runner runner(diagnostics);
  for (auto it = begin; it != end; ++it, counter.increment()) {
    auto &mutant = *it;
    ExecutionResult result;
    if (mutant->isCovered()) {
      result = runner.runProgram(executable,
                                 extraArgs,
                                 { mutant->getIdentifier() },
                                 baseline.runningTime * 10,
                                 configuration.captureMutantOutput,
                                 std::nullopt);
    } else {
      result.status = NotCovered;
    }
    storage.push_back(std::make_unique<MutationResult>(result, mutant.get()));
  }
}

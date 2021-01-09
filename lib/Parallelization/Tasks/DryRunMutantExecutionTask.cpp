#include "mull/Parallelization/Tasks/DryRunMutantExecutionTask.h"
#include "mull/Parallelization/Progress.h"

using namespace mull;

void DryRunMutantExecutionTask::operator()(iterator begin, iterator end, Out &storage,
                                           progress_counter &counter) {
  for (auto it = begin; it != end; it++, counter.increment()) {
    auto &mutant = *it;
    ExecutionResult result;
    result.status = DryRun;
    storage.push_back(std::make_unique<MutationResult>(result, mutant.get()));
  }
}

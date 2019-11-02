#include "mull/Parallelization/Tasks/DryRunMutantExecutionTask.h"

#include "mull/Parallelization/Progress.h"
#include <llvm/ADT/STLExtras.h>

using namespace mull;
using namespace llvm;

void DryRunMutantExecutionTask::operator()(iterator begin, iterator end,
                                           Out &storage,
                                           progress_counter &counter) {
  for (auto it = begin; it != end; it++, counter.increment()) {
    auto mutationPoint = *it;
    for (auto &reachableTest : mutationPoint->getReachableTests()) {
      auto test = reachableTest.first;
      auto distance = reachableTest.second;
      auto timeout = test->getExecutionResult().runningTime * 10;
      ExecutionResult result;
      result.status = DryRun;
      result.runningTime = timeout;
      storage.push_back(
          make_unique<MutationResult>(result, mutationPoint, distance, test));
    }
  }
}

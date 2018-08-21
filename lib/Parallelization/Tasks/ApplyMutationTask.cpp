#include "Parallelization/Tasks/ApplyMutationTask.h"
#include "Parallelization/Progress.h"
#include "MutationPoint.h"

using namespace mull;

void ApplyMutationTask::operator()(iterator begin,
                                   iterator end,
                                   Out &storage,
                                   progress_counter &counter) {
  for (auto it = begin; it != end; ++it, counter.increment()) {
    auto point = *it;
    point->applyMutation();
  }
}

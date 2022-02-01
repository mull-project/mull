#include "mull/Parallelization/Tasks/ApplyMutationTask.h"

#include "mull/MutationPoint.h"
#include "mull/Parallelization/Progress.h"

using namespace mull;

void ApplyMutationTask::operator()(iterator begin, iterator end, Out &storage,
                                   progress_counter &counter) {
  for (auto it = begin; it != end; ++it, counter.increment()) {
    auto point = *it;
    point->recordMutation();
    point->applyMutation();
  }
}

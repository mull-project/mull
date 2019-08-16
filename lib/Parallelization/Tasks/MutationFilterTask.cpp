#include "mull/Parallelization/Tasks/MutationFilterTask.h"

#include "mull/MutationFilters/MutationFilter.h"
#include "mull/Parallelization/Progress.h"

using namespace mull;

MutationFilterTask::MutationFilterTask(MutationFilter &filter)
    : filter(filter) {}

void MutationFilterTask::operator()(iterator begin, iterator end, Out &storage,
                                    progress_counter &counter) {
  for (auto it = begin; it != end; ++it, counter.increment()) {
    auto point = *it;
    if (!filter.shouldSkip(point)) {
      storage.push_back(point);
    }
  }
}

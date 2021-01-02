#include "mull/Parallelization/Tasks/FunctionFilterTask.h"

#include "mull/Filters/FunctionFilter.h"
#include "mull/Parallelization/Progress.h"
#include <cassert>

using namespace mull;

FunctionFilterTask::FunctionFilterTask(FunctionFilter &filter)
    : filter(filter) {}

void FunctionFilterTask::operator()(iterator begin, iterator end, Out &storage,
                                    progress_counter &counter) {
  for (auto it = begin; it != end; ++it, counter.increment()) {
    FunctionUnderTest &functionUnderTest = *it;
    assert(functionUnderTest.getFunction());
    if (!filter.shouldSkip(functionUnderTest.getFunction())) {
      storage.push_back(std::move(functionUnderTest));
    }
  }
}

#include "mull/Parallelization/Tasks/InstructionSelectionTask.h"
#include "mull/FunctionUnderTest.h"
#include "mull/Parallelization/Progress.h"
#include <cassert>

using namespace mull;

InstructionSelectionTask::InstructionSelectionTask(
    const std::vector<InstructionFilter *> &filters)
    : filters(filters) {}

void InstructionSelectionTask::operator()(iterator begin, iterator end,
                                          Out &storage,
                                          progress_counter &counter) {
  for (auto it = begin; it != end; ++it, counter.increment()) {
    FunctionUnderTest &functionUnderTest = *it;
    assert(functionUnderTest.getFunction());
    functionUnderTest.selectInstructions(filters);
  }
}

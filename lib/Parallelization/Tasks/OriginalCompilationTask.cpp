#include "mull/Parallelization/Tasks/OriginalCompilationTask.h"

#include "mull/Config/Configuration.h"
#include "mull/Parallelization/Progress.h"
#include "mull/Toolchain/Toolchain.h"

using namespace mull;
using namespace llvm;

OriginalCompilationTask::OriginalCompilationTask(Toolchain &toolchain) : toolchain(toolchain) {}

void OriginalCompilationTask::operator()(iterator begin, iterator end, Out &storage,
                                         progress_counter &counter) {
  for (auto it = begin; it != end; it++, counter.increment()) {
    std::string objectFilename = toolchain.compiler().compileBitcode((**it));
    storage.push_back(objectFilename);
  }
}

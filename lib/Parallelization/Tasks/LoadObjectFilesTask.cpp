#include "mull/Parallelization/Tasks/LoadObjectFilesTask.h"
#include "mull/Diagnostics/Diagnostics.h"
#include "mull/Parallelization/Progress.h"

using namespace mull;
using namespace llvm;

LoadObjectFilesTask::LoadObjectFilesTask(Diagnostics &diagnostics) : diagnostics(diagnostics) {}

void LoadObjectFilesTask::operator()(iterator begin, iterator end, Out &storage,
                                     progress_counter &counter) {
  for (auto it = begin; it != end; it++, counter.increment()) {
    auto objectFilePath = *it;
    auto objectFile = llvm::object::ObjectFile::createObjectFile(objectFilePath);
    if (!objectFile) {
      diagnostics.warning(std::string("Cannot load object file: ") + objectFilePath);
      continue;
    }
    storage.emplace_back(std::move(objectFile.get()));
  }
}

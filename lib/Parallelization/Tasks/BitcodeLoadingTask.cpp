#include "mull/Parallelization/Tasks/BitcodeLoadingTask.h"

#include "mull/BitcodeLoader.h"
#include "mull/Parallelization/Parallelization.h"

using namespace mull;
using namespace llvm;

BitcodeLoadingTask::BitcodeLoadingTask(Diagnostics &diagnostics, BitcodeLoader &loader)
    : diagnostics(diagnostics), loader(loader) {}

void BitcodeLoadingTask::operator()(iterator begin, iterator end, Out &storage,
                                    progress_counter &counter) {
  for (auto it = begin; it != end; ++it, counter.increment()) {
    auto bitcode = loader.loadBitcodeAtPath(*it, diagnostics);
    if (bitcode != nullptr) {
      storage.push_back(std::move(bitcode));
    }
  }
}

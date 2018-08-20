#include "Parallelization/Parallelization.h"
#include "Parallelization/Tasks/ModuleLoadingTask.h"
#include "ModuleLoader.h"

using namespace mull;
using namespace llvm;

ModuleLoadingTask::ModuleLoadingTask(LLVMContext &context, ModuleLoader &loader)
    : context(context), loader(loader) {}

void ModuleLoadingTask::operator()(iterator begin, iterator end, Out &storage, progress_counter &counter) {
  for (auto it = begin; it != end; ++it, counter.increment()) {
    auto module = loader.loadModuleAtPath(*it, context);
    if (module != nullptr) {
      storage.push_back(std::move(module));
    }
  }
}

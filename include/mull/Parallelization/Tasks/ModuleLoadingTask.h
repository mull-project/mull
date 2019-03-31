#pragma once

#include "mull/MullModule.h"

namespace llvm {
class LLVMContext;
}

namespace mull {

class ModuleLoader;
class progress_counter;

class ModuleLoadingTask {
public:
  using In = const std::vector<std::string>;
  using Out = std::vector<std::unique_ptr<MullModule>>;
  using iterator = In::const_iterator;

  ModuleLoadingTask(llvm::LLVMContext &context, ModuleLoader &loader);
  void operator()(iterator begin, iterator end, Out &storage,
                  progress_counter &counter);

private:
  llvm::LLVMContext &context;
  ModuleLoader &loader;
};

} // namespace mull

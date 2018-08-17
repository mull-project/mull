#include "Parallelization/Tasks/OriginalCompilationTask.h"
#include "Parallelization/Progress.h"
#include "Toolchain/Toolchain.h"

#include <llvm/ExecutionEngine/ExecutionEngine.h>
#include <llvm/Support/TargetSelect.h>

using namespace mull;
using namespace llvm;

OriginalCompilationTask::OriginalCompilationTask(Toolchain &toolchain) : toolchain(toolchain) {}

void mull::OriginalCompilationTask::operator()(mull::OriginalCompilationTask::iterator begin,
                                             mull::OriginalCompilationTask::iterator end,
                                             mull::OriginalCompilationTask::Out &storage,
                                             progress_counter &counter) {
  EngineBuilder builder;
  auto target = builder.selectTarget(llvm::Triple(), "", "",
                                     llvm::SmallVector<std::string, 1>());
  std::unique_ptr<TargetMachine> localMachine(target);

  for (auto it = begin; it != end; it++, counter.increment()) {
    auto &module = *it->get();

    auto objectFile = toolchain.cache().getObject(module);
    if (objectFile.getBinary() == nullptr) {
      LLVMContext localContext;
      auto clonedModule = module.clone(localContext);
      objectFile = toolchain.compiler().compileModule(*clonedModule, *localMachine);
      toolchain.cache().putObject(objectFile, module);
    }

    storage.push_back(std::move(objectFile));
  }
}

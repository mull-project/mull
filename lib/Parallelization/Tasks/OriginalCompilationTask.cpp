#include "mull/Parallelization/Tasks/OriginalCompilationTask.h"

#include "mull/Parallelization/Progress.h"
#include "mull/Toolchain/Toolchain.h"

#include <llvm/ExecutionEngine/ExecutionEngine.h>
#include <llvm/Support/TargetSelect.h>

using namespace mull;
using namespace llvm;

OriginalCompilationTask::OriginalCompilationTask(Toolchain &toolchain)
    : toolchain(toolchain) {}

void OriginalCompilationTask::operator()(iterator begin, iterator end,
                                         Out &storage,
                                         progress_counter &counter) {
  EngineBuilder builder;
  auto target = builder.selectTarget(llvm::Triple(), "", "",
                                     llvm::SmallVector<std::string, 1>());
  std::unique_ptr<TargetMachine> localMachine(target);

  for (auto it = begin; it != end; it++, counter.increment()) {
    auto &module = *it->get();

    auto objectFile = toolchain.cache().getObject(module);
    if (objectFile.getBinary() == nullptr) {
      objectFile = toolchain.compiler().compileModule(module, *localMachine);
      toolchain.cache().putObject(objectFile, module);
    }

    storage.push_back(std::move(objectFile));
  }
}

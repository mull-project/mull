#include "mull/Parallelization/Tasks/InstrumentedCompilationTask.h"

#include "mull/Instrumentation/Instrumentation.h"
#include "mull/Parallelization/Progress.h"
#include "mull/Toolchain/Toolchain.h"

#include <llvm/ExecutionEngine/ExecutionEngine.h>
#include <llvm/Support/TargetSelect.h>

using namespace mull;
using namespace llvm;

InstrumentedCompilationTask::InstrumentedCompilationTask(
    Instrumentation &instrumentation, Toolchain &toolchain)
    : instrumentation(instrumentation), toolchain(toolchain) {}

void InstrumentedCompilationTask::operator()(iterator begin, iterator end,
                                             Out &storage,
                                             progress_counter &counter) {
  EngineBuilder builder;
  auto target = builder.selectTarget(llvm::Triple(), "", "",
                                     llvm::SmallVector<std::string, 1>());
  std::unique_ptr<TargetMachine> localMachine(target);

  for (auto it = begin; it != end; it++, counter.increment()) {
    auto &module = *it->get();
    auto objectFile = toolchain.cache().getInstrumentedObject(module);
    if (objectFile.getBinary() == nullptr) {
      LLVMContext instrumentationContext;
      auto clonedModule = module.clone(instrumentationContext);

      instrumentation.insertCallbacks(clonedModule->getModule());
      objectFile =
          toolchain.compiler().compileModule(*clonedModule, *localMachine);
      toolchain.cache().putInstrumentedObject(objectFile, module);
    }
    storage.push_back(std::move(objectFile));
  }
}

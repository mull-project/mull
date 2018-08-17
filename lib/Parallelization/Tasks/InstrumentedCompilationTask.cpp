#include "Parallelization/Tasks/InstrumentedCompilationTask.h"
#include "Parallelization/Progress.h"
#include "Toolchain/Toolchain.h"
#include "Instrumentation/Instrumentation.h"

#include <llvm/ExecutionEngine/ExecutionEngine.h>
#include <llvm/Support/TargetSelect.h>

using namespace mull;
using namespace llvm;

InstrumentedCompilationTask::InstrumentedCompilationTask(Instrumentation &instrumentation, Toolchain &toolchain)
    : instrumentation(instrumentation), toolchain(toolchain) {}

void mull::InstrumentedCompilationTask::operator()(mull::InstrumentedCompilationTask::iterator begin,
                                                   mull::InstrumentedCompilationTask::iterator end,
                                                   mull::InstrumentedCompilationTask::Out &storage,
                                                   mull::progress_counter &counter) {
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
      objectFile = toolchain.compiler().compileModule(*clonedModule, *localMachine);
      toolchain.cache().putInstrumentedObject(objectFile, module);
    }
    storage.push_back(std::move(objectFile));
  }
}

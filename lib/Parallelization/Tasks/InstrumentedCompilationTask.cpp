#include "mull/Parallelization/Tasks/InstrumentedCompilationTask.h"

#include "mull/Instrumentation/Instrumentation.h"
#include "mull/Parallelization/Progress.h"
#include "mull/Toolchain/Toolchain.h"

#include <llvm/ExecutionEngine/ExecutionEngine.h>
#include <llvm/Support/TargetSelect.h>

using namespace mull;
using namespace llvm;

InstrumentedCompilationTask::InstrumentedCompilationTask(Diagnostics &diagnostics,
                                                         Instrumentation &instrumentation,
                                                         Toolchain &toolchain)
    : diagnostics(diagnostics), instrumentation(instrumentation), toolchain(toolchain) {}

void InstrumentedCompilationTask::operator()(iterator begin, iterator end,
                                             Out &storage,
                                             progress_counter &counter) {
  EngineBuilder builder;
  auto target = builder.selectTarget(llvm::Triple(), "", "",
                                     llvm::SmallVector<std::string, 1>());
  std::unique_ptr<TargetMachine> localMachine(target);

  for (auto it = begin; it != end; it++, counter.increment()) {
    auto &bitcode = **it;
    auto objectFile = toolchain.cache().getInstrumentedObject(bitcode);
    if (objectFile.getBinary() == nullptr) {
      LLVMContext instrumentationContext;
      auto clonedBitcode = bitcode.clone(instrumentationContext, diagnostics);

      instrumentation.insertCallbacks(clonedBitcode->getModule());
      objectFile =
          toolchain.compiler().compileBitcode(*clonedBitcode, *localMachine);
      toolchain.cache().putInstrumentedObject(objectFile, bitcode);
    }
    storage.push_back(std::move(objectFile));
  }
}

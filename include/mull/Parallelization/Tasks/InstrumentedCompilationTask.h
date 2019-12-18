#pragma once

#include "mull/Bitcode.h"

#include <llvm/Object/ObjectFile.h>
#include <vector>

namespace mull {

class Toolchain;
class Instrumentation;
class progress_counter;

class InstrumentedCompilationTask {
public:
  using In = std::vector<std::unique_ptr<Bitcode>>;
  using Out = std::vector<llvm::object::OwningBinary<llvm::object::ObjectFile>>;
  using iterator = In::const_iterator;

  InstrumentedCompilationTask(Diagnostics &diagnostics, Instrumentation &instrumentation,
                              Toolchain &toolchain);

  void operator()(iterator begin, iterator end, Out &storage,
                  progress_counter &counter);

private:
  Diagnostics &diagnostics;
  Instrumentation &instrumentation;
  Toolchain &toolchain;
};
} // namespace mull

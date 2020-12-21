#pragma once

#include "mull/Toolchain/Compiler.h"
#include "mull/Toolchain/Linker.h"
#include "mull/Toolchain/Mangler.h"
#include "mull/Toolchain/ObjectCache.h"

#include <llvm/Target/TargetMachine.h>

namespace mull {
struct Configuration;
class Diagnostics;

class Toolchain {

  class NativeTarget {
  public:
    NativeTarget();
  };

  NativeTarget nativeTarget;
  std::unique_ptr<llvm::TargetMachine> machine;
  ObjectCache objectCache;
  Compiler simpleCompiler;
  Linker objectLinker;
  Mangler nameMangler;

public:
  Toolchain(Diagnostics &diagnostics, const Configuration &config);

  ObjectCache &cache();
  Compiler &compiler();
  Linker &linker();
  llvm::TargetMachine &targetMachine();
  mull::Mangler &mangler();
};
} // namespace mull

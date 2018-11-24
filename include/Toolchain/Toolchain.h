#pragma once

#include "Toolchain/ObjectCache.h"
#include "Toolchain/Compiler.h"
#include "Toolchain/Mangler.h"

#include <llvm/Target/TargetMachine.h>

namespace mull {
  class RawConfig;

  class Toolchain {

    class NativeTarget {
      public:
      NativeTarget();
    };

    NativeTarget nativeTarget;
    std::unique_ptr<llvm::TargetMachine> machine;
    ObjectCache objectCache;
    Compiler simpleCompiler;
    Mangler nameMangler;
  public:
    explicit Toolchain(RawConfig &config);

    ObjectCache &cache();
    Compiler &compiler();
    llvm::TargetMachine &targetMachine();
    mull::Mangler &mangler();
  };
}

#pragma once

#include "Toolchain/ObjectCache.h"
#include "Toolchain/Compiler.h"

#include "llvm/Target/TargetMachine.h" 

namespace mull {
  class Config;

  class Toolchain {

    class NativeTarget {
      public:
      NativeTarget();
    };

    NativeTarget nativeTarget;
    std::unique_ptr<llvm::TargetMachine> machine;
    ObjectCache objectCache;
    Compiler simpleCompiler;
  public:
    Toolchain(Config &config);

    ObjectCache &cache();
    Compiler &compiler();
    llvm::TargetMachine &targetMachine();
  };
}

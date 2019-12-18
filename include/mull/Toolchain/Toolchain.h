#pragma once

#include "Compiler.h"
#include "Mangler.h"
#include "ObjectCache.h"

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
  Mangler nameMangler;

public:
  Toolchain(Diagnostics &diagnostics, const Configuration &config);

  ObjectCache &cache();
  Compiler &compiler();
  llvm::TargetMachine &targetMachine();
  mull::Mangler &mangler();
};
} // namespace mull

#pragma once

#include "mull/Toolchain/Compiler.h"
#include "mull/Toolchain/Linker.h"

namespace mull {
struct Configuration;
class Diagnostics;

class Toolchain {
  Compiler simpleCompiler;
  Linker objectLinker;

public:
  Toolchain(Diagnostics &diagnostics, const Configuration &config);

  Compiler &compiler();
  Linker &linker();
};
} // namespace mull

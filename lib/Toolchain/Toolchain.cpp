#include "mull/Toolchain/Toolchain.h"
#include "mull/Config/Configuration.h"

using namespace mull;

Toolchain::Toolchain(Diagnostics &diagnostics, const Configuration &config)
    : simpleCompiler(diagnostics, config), objectLinker(config, diagnostics) {}

Compiler &Toolchain::compiler() {
  return simpleCompiler;
}

Linker &Toolchain::linker() {
  return objectLinker;
}

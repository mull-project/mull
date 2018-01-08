#pragma once

#include <cstdint>

namespace llvm {
  class Function;
}

namespace mull {
  class Driver;
  class Toolchain;

  extern "C" void mull_enterFunction(Driver *driver, uint64_t functionIndex);
  extern "C" void mull_leaveFunction(Driver *driver, uint64_t functionIndex);

  class Callbacks {
  public:
    explicit Callbacks(Toolchain &t);
    void injectCallbacks(llvm::Function *function, uint64_t index, Driver *driver);
  private:
    Toolchain &toolchain;
  };
}

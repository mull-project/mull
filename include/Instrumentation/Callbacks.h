#pragma once

#include <cstdint>

namespace llvm {
  class Function;
}

namespace mull {
  class Toolchain;
  class InstrumentationInfo;

  extern "C" void mull_enterFunction(InstrumentationInfo *info, uint64_t functionIndex);
  extern "C" void mull_leaveFunction(InstrumentationInfo *info, uint64_t functionIndex);

  class Callbacks {
  public:
    explicit Callbacks(Toolchain &t);
    void injectCallbacks(llvm::Function *function, uint64_t index, InstrumentationInfo &info);
  private:
    Toolchain &toolchain;
  };
}

#pragma once

#include <cstdint>

namespace llvm {
  class Function;
  class Module;
  class Value;
}

namespace mull {

  extern "C" void mull_enterFunction(void **trampoline, uint32_t functionIndex);
  extern "C" void mull_leaveFunction(void **trampoline, uint32_t functionIndex);

  class Callbacks {
  public:
    void injectCallbacks(llvm::Function *function,
                         uint32_t index,
                         llvm::Value *infoPointer,
                         llvm::Value *offset);

    llvm::Value *injectInstrumentationInfoPointer(llvm::Module *module,
                                                  const char *variableName);

    llvm::Value *injectFunctionIndexOffset(llvm::Module *module,
                                           const char *functionIndexOffsetPrefix);
  };
}

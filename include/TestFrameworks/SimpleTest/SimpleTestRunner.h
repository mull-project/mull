#pragma once

#include "TestFrameworks/TestRunner.h"
#include "LLVMCompatibility.h"

#include <llvm/ExecutionEngine/Orc/ExecutionUtils.h>

namespace llvm {

class Function;

}

namespace mull {
class Mangler;
class Test;
class JITEngine;
struct InstrumentationInfo;

class SimpleTestRunner : public TestRunner {
  Mangler &mangler;
  llvm_compat::CXXRuntimeOverrides overrides;
  InstrumentationInfo **trampoline;
public:
  explicit SimpleTestRunner(Mangler &mangler);
  ~SimpleTestRunner() override;

  void loadInstrumentedProgram(ObjectFiles &objectFiles,
                               Instrumentation &instrumentation,
                               JITEngine &jit) override;
  void loadMutatedProgram(ObjectFiles &objectFiles, Trampolines &trampolines, JITEngine &jit) override;
  ExecutionStatus runTest(Test *test, JITEngine &jit) override;

private:
  void *functionPointer(const llvm::Function &function, JITEngine &jit);
};

}

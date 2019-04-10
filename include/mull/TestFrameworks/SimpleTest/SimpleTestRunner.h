#pragma once

#include "LLVMCompatibility.h"
#include "mull/TestFrameworks/TestRunner.h"

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
  void loadMutatedProgram(ObjectFiles &objectFiles, Trampolines &trampolines,
                          JITEngine &jit) override;
  ExecutionStatus runTest(JITEngine &jit, Program &program,
                          Test *test) override;

private:
  void *functionPointer(const llvm::Function &function, JITEngine &jit);
};

} // namespace mull

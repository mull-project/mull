#pragma once

#include "TestRunner.h"
#include "Mangler.h"
#include "Toolchain/JITEngine.h"

#include <llvm/ExecutionEngine/Orc/ExecutionUtils.h>

namespace llvm {

class Function;
class TargetMachine;

}

namespace mull {

class Test;
struct InstrumentationInfo;

class SimpleTestRunner : public TestRunner {
  Mangler mangler;
  llvm::orc::LocalCXXRuntimeOverrides overrides;
  InstrumentationInfo **trampoline;
public:
  explicit SimpleTestRunner(llvm::TargetMachine &targetMachine);
  ~SimpleTestRunner() override;

  void loadInstrumentedProgram(ObjectFiles &objectFiles,
                               Instrumentation &instrumentation,
                               JITEngine &jit) override;
  void loadProgram(ObjectFiles &objectFiles, JITEngine &jit) override;
  ExecutionStatus runTest(Test *test, JITEngine &jit) override;

private:
  void *functionPointer(const llvm::Function &function, JITEngine &jit);
};

}

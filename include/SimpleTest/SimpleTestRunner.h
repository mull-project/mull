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
  JITEngine jit;
  Mangler mangler;
  llvm::orc::LocalCXXRuntimeOverrides overrides;
  InstrumentationInfo **trampoline;
public:
  SimpleTestRunner(llvm::TargetMachine &targetMachine);
  ~SimpleTestRunner() override;
  void loadInstrumentedProgram(ObjectFiles &objectFiles,
                               Instrumentation &instrumentation) override;
  void loadProgram(ObjectFiles &objectFiles) override;
  ExecutionStatus runTest(Test *test) override;

private:
  void *TestFunctionPointer(const llvm::Function &Function);
};

}

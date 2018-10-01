#pragma once

#include "TestRunner.h"
#include "Toolchain/JITEngine.h"

#include <llvm/ExecutionEngine/Orc/ExecutionUtils.h>
#include <llvm/Object/Binary.h>
#include <llvm/Object/ObjectFile.h>
#include <llvm/Target/TargetMachine.h>

namespace llvm {

class Function;
class Module;

}

namespace mull {

class Instrumentation;
class Mangler;
struct InstrumentationInfo;

class CustomTestRunner : public TestRunner {
  Mangler &mangler;
  llvm::orc::LocalCXXRuntimeOverrides overrides;
  InstrumentationInfo **trampoline;
public:

  CustomTestRunner(Mangler &mangler);
  ~CustomTestRunner() override;

  void loadInstrumentedProgram(ObjectFiles &objectFiles,
                               Instrumentation &instrumentation,
                               JITEngine &jit) override;
  void loadProgram(ObjectFiles &objectFiles, JITEngine &jit) override;
  ExecutionStatus runTest(Test *test, JITEngine &jit) override;

private:
  void *getConstructorPointer(const llvm::Function &function, JITEngine &jit);
  void *getFunctionPointer(const std::string &functionName, JITEngine &jit);
  void runStaticConstructor(llvm::Function *function, JITEngine &jit);
};

}

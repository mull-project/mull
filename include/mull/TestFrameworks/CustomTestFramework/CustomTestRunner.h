#pragma once

#include "LLVMCompatibility.h"
#include "mull/TestFrameworks/TestRunner.h"
#include "mull/Toolchain/JITEngine.h"

#include <llvm/ExecutionEngine/Orc/ExecutionUtils.h>
#include <llvm/Object/Binary.h>
#include <llvm/Object/ObjectFile.h>
#include <llvm/Target/TargetMachine.h>

namespace llvm {

class Function;
class Module;

} // namespace llvm

namespace mull {

class Instrumentation;
class Mangler;
struct InstrumentationInfo;

class CustomTestRunner : public TestRunner {
  Mangler &mangler;
  llvm_compat::CXXRuntimeOverrides overrides;
  InstrumentationInfo **trampoline;

public:
  explicit CustomTestRunner(Mangler &mangler);
  ~CustomTestRunner() override;

  void loadInstrumentedProgram(ObjectFiles &objectFiles,
                               Instrumentation &instrumentation,
                               JITEngine &jit) override;
  void loadMutatedProgram(ObjectFiles &objectFiles, Trampolines &trampolines,
                          JITEngine &jit) override;
  ExecutionStatus runTest(Test *test, JITEngine &jit) override;

private:
  void *getConstructorPointer(const llvm::Function &function, JITEngine &jit);
  void *getFunctionPointer(const std::string &functionName, JITEngine &jit);
  void runStaticConstructor(llvm::Function *function, JITEngine &jit);
};

} // namespace mull

#pragma once

#include "mull/TestFrameworks/TestRunner.h"

namespace mull {

class Mangler;
class JITEngine;
class Mangler;
struct InstrumentationInfo;

class NativeTestRunner : public TestRunner {
public:
  explicit NativeTestRunner(Mangler &mangler);
  ~NativeTestRunner() override;

  void loadInstrumentedProgram(ObjectFiles &objectFiles,
                               Instrumentation &instrumentation,
                               JITEngine &jit) override;
  void loadMutatedProgram(ObjectFiles &objectFiles, Trampolines &trampolines,
                          JITEngine &jit) override;
  ExecutionStatus runTest(JITEngine &jit, Program &program,
                          Test &test) override;

private:
  Mangler &mangler;
  llvm_compat::CXXRuntimeOverrides overrides;
  InstrumentationInfo **trampoline;

  void *getConstructorPointer(const llvm::Function &function, JITEngine &jit);
  void *getFunctionPointer(const std::string &functionName, JITEngine &jit);

  void runStaticConstructor(llvm::Function *constructor, JITEngine &jit);
};

} // namespace mull
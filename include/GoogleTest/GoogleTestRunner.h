#pragma once

#include "TestRunner.h"

#include "Mangler.h"
#include "Toolchain/JITEngine.h"

#include <llvm/ExecutionEngine/Orc/ExecutionUtils.h>
#include <llvm/Object/Binary.h>
#include <llvm/Object/ObjectFile.h>
#include <llvm/Target/TargetMachine.h>

namespace llvm {

class Function;

}

namespace mull {

struct InstrumentationInfo;

class GoogleTestRunner : public TestRunner {
  Mangler mangler;
  llvm::orc::LocalCXXRuntimeOverrides overrides;

  std::string fGoogleTestInit;
  std::string fGoogleTestInstance;
  std::string fGoogleTestRun;
  InstrumentationInfo **trampoline;
public:

  GoogleTestRunner(llvm::TargetMachine &machine);
  ~GoogleTestRunner();

  void loadInstrumentedProgram(ObjectFiles &objectFiles, Instrumentation &instrumentation, JITEngine &jit) override;
  void loadProgram(ObjectFiles &objectFiles, JITEngine &jit) override;
  ExecutionStatus runTest(Test *test, JITEngine &jit) override;

private:
  void *getConstructorPointer(const llvm::Function &function, JITEngine &jit);
  void *getFunctionPointer(const std::string &functionName, JITEngine &jit);

  void runStaticConstructor(llvm::Function *constructor, JITEngine &jit);
};

}

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
  JITEngine jit;
  Mangler mangler;
  llvm::orc::LocalCXXRuntimeOverrides overrides;

  std::string fGoogleTestInit;
  std::string fGoogleTestInstance;
  std::string fGoogleTestRun;
  InstrumentationInfo **trampoline;
public:

  GoogleTestRunner(llvm::TargetMachine &machine);
  ~GoogleTestRunner();

  void loadInstrumentedProgram(ObjectFiles &objectFiles, Instrumentation &instrumentation) override;
  void loadProgram(ObjectFiles &objectFiles) override;
  ExecutionStatus runTest(Test *test) override;

private:
  void *GetCtorPointer(const llvm::Function &Function);
  void *getFunctionPointer(const std::string &functionName);

  void runStaticCtor(llvm::Function *Ctor);
};

}

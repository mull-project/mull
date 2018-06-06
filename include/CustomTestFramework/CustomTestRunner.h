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
class Module;

}

namespace mull {
class Instrumentation;
struct InstrumentationInfo;

class CustomTestRunner : public TestRunner {
  JITEngine jit;
  Mangler mangler;
  llvm::orc::LocalCXXRuntimeOverrides overrides;
  InstrumentationInfo **trampoline;
public:

  CustomTestRunner(llvm::TargetMachine &machine);
  ~CustomTestRunner();

  void loadInstrumentedProgram(ObjectFiles &objectFiles, Instrumentation &instrumentation) override;
  void loadProgram(ObjectFiles &objectFiles) override;
  ExecutionStatus runTest(Test *test) override;

private:
  void *GetCtorPointer(const llvm::Function &Function);
  void *getFunctionPointer(const std::string &functionName);
  void runStaticCtor(llvm::Function *Ctor);
};

}

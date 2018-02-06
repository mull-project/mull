#pragma once

#include "TestRunner.h"
#include "Mangler.h"

#include <llvm/ExecutionEngine/Orc/ExecutionUtils.h>
#include <llvm/ExecutionEngine/Orc/ObjectLinkingLayer.h>

namespace llvm {

class Function;
class TargetMachine;

}

namespace mull {

class Test;
struct InstrumentationInfo;

class SimpleTestRunner : public TestRunner {
  llvm::orc::ObjectLinkingLayer<> ObjectLayer;
  llvm::orc::ObjectLinkingLayer<>::ObjSetHandleT handle;
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

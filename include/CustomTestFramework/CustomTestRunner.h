#pragma once

#include "TestRunner.h"

#include "Mangler.h"

#include <llvm/ExecutionEngine/Orc/ExecutionUtils.h>
#include <llvm/ExecutionEngine/Orc/ObjectLinkingLayer.h>
#include <llvm/IR/Mangler.h>
#include <llvm/Object/Binary.h>
#include <llvm/Object/ObjectFile.h>
#include <llvm/Target/TargetMachine.h>

namespace llvm {

class Function;
class Module;

}

namespace mull {
struct InstrumentationInfo;

class CustomTestRunner : public TestRunner {
  llvm::orc::ObjectLinkingLayer<> ObjectLayer;
  mull::Mangler mangler;
  llvm::orc::LocalCXXRuntimeOverrides overrides;
  std::string instrumentationInfoName;
  llvm::orc::ObjectLinkingLayer<>::ObjSetHandleT handle;
  InstrumentationInfo **trampoline;
public:

  CustomTestRunner(llvm::TargetMachine &machine);
  ~CustomTestRunner();
  void loadProgram(ObjectFiles &objectFiles) override;
  ExecutionStatus runTest(Test *test) override;

private:
  void *GetCtorPointer(const llvm::Function &Function);
  void *getFunctionPointer(const std::string &functionName);
  void runStaticCtor(llvm::Function *Ctor);
};

}

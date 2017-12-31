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

class GoogleTestRunner : public TestRunner {
  llvm::orc::ObjectLinkingLayer<> ObjectLayer;
  mull::Mangler mangler;
  llvm::orc::LocalCXXRuntimeOverrides overrides;

  std::string fGoogleTestInit;
  std::string fGoogleTestInstance;
  std::string fGoogleTestRun;
  llvm::orc::ObjectLinkingLayer<>::ObjSetHandleT handle;
public:

  GoogleTestRunner(llvm::TargetMachine &machine);
  ExecutionStatus runTest(Test *test, ObjectFiles &objectFiles) override;

private:
  void *GetCtorPointer(const llvm::Function &Function);
  void *getFunctionPointer(const std::string &functionName);

  void runStaticCtor(llvm::Function *Ctor);
};

}

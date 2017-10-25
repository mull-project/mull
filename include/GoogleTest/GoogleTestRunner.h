#pragma once

#include "TestRunner.h"

#include "llvm/ExecutionEngine/Orc/ObjectLinkingLayer.h"
#include "llvm/IR/Mangler.h"
#include "llvm/Object/Binary.h"
#include "llvm/Object/ObjectFile.h"
#include "llvm/Target/TargetMachine.h"

namespace llvm {

class Function;
class Module;

}

namespace mull {

  class GoogleTestRunner : public TestRunner {
  llvm::orc::ObjectLinkingLayer<> ObjectLayer;
  llvm::Mangler Mangler;
public:

  GoogleTestRunner(llvm::TargetMachine &machine);
  ExecutionResult runTest(Test *Test, ObjectFiles &ObjectFiles) override;

private:
  std::string getNameWithPrefix(const std::string &name);

  void *GetCtorPointer(const llvm::Function &Function);
  void *getFunctionPointer(const std::string &functionName);

  void runStaticCtor(llvm::Function *Ctor);
};

}

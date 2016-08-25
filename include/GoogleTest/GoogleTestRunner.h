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

namespace Mutang {

  class GoogleTestRunner : public  TestRunner {
  llvm::orc::ObjectLinkingLayer<> ObjectLayer;
  std::unique_ptr<llvm::TargetMachine> TM;
  llvm::Mangler Mangler;
public:

  GoogleTestRunner();
  ExecutionResult runTest(Test *Test, ObjectFiles &ObjectFiles) override;

private:
  std::string MangleName(const llvm::StringRef &Name);
  void *GetCtorPointer(const llvm::Function &Function);
  void *FunctionPointer(const char *FunctionName);

  void runStaticCtor(llvm::Function *Ctor);
};

}

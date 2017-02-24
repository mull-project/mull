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

  class RustTestRunner : public TestRunner {
  llvm::orc::ObjectLinkingLayer<> objectLayer;

public:

  RustTestRunner(llvm::TargetMachine &machine);
  ExecutionResult runTest(Test *Test, ObjectFiles &objectFiles) override;

private:
    void *FunctionPointer(const char *functionName);

};

}

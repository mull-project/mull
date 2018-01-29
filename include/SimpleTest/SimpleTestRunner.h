#pragma once

#include "TestRunner.h"

#include <llvm/ExecutionEngine/Orc/ObjectLinkingLayer.h>
#include <llvm/IR/Mangler.h>

namespace llvm {

class Function;
class TargetMachine;

}

namespace mull {

class Test;
struct InstrumentationInfo;

class SimpleTestRunner : public TestRunner {
  llvm::orc::ObjectLinkingLayer<> ObjectLayer;
  llvm::Mangler Mangler;
  llvm::orc::ObjectLinkingLayer<>::ObjSetHandleT handle;
  InstrumentationInfo **trampoline;
public:
  SimpleTestRunner(llvm::TargetMachine &targetMachine);
  ~SimpleTestRunner() override;
  void loadProgram(ObjectFiles &objectFiles) override;
  ExecutionStatus runTest(Test *test) override;

private:
  std::string MangleName(const llvm::StringRef &Name);
  void *TestFunctionPointer(const llvm::Function &Function);
};

}

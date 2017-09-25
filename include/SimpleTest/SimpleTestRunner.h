#pragma once

#include "TestRunner.h"

#include <llvm/ExecutionEngine/Orc/ObjectLinkingLayer.h>
#include <llvm/IR/Mangler.h>
#include <llvm/Target/TargetMachine.h>

namespace llvm {

class Function;
class TargetMachine;

}

namespace mull {

class Test;

class SimpleTestRunner : public TestRunner {
  llvm::Mangler Mangler;
public:
  SimpleTestRunner(TargetMachine &machine, MullJIT &jit);
  std::unique_ptr<RuntimeDyld::SymbolResolver> resolver() override;
  std::unique_ptr<SectionMemoryManager> memoryManager() override;
  ExecutionResult runTest(Test *test) override;

private:
  std::string MangleName(const llvm::StringRef &Name);
  void *TestFunctionPointer(const llvm::Function &Function);
};

}

#pragma once

#include "TestRunner.h"
#include "MullJIT.h"

#include "llvm/ExecutionEngine/Orc/ObjectLinkingLayer.h"
#include "llvm/ExecutionEngine/Orc/IRCompileLayer.h"
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
  llvm::Mangler Mangler;
  pid_t creatorPID;
public:
  GoogleTestRunner(llvm::TargetMachine &machine, MullJIT &jit);

  std::unique_ptr<RuntimeDyld::SymbolResolver> resolver() override;
  std::unique_ptr<SectionMemoryManager> memoryManager() override;
  ExecutionResult runTest(Test *Test) override;

private:
  std::string MangleName(const llvm::StringRef &Name);
  void *GetCtorPointer(const llvm::Function &Function);
  void *FunctionPointer(const char *FunctionName);

  void runStaticCtor(llvm::Function *Ctor);
};

}

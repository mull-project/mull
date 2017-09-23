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
  MullJIT jit;
  pid_t creatorPID;
public:

  GoogleTestRunner(llvm::TargetMachine &machine);
  ExecutionResult runTest(Test *Test, ObjectFiles &ObjectFiles) override;

  ExecutionResult runTest(Test *Test) override;

  DynamicCallTree *dynamicCallTree() override;
  std::unique_ptr<CallTree> callTree() override;
  void cleanupCallTree(std::unique_ptr<CallTree> root) override;
  void prepareForExecution(std::vector<llvm::Module *> &modules) override;

  virtual ~GoogleTestRunner() {}

private:
  std::string MangleName(const llvm::StringRef &Name);
  void *GetCtorPointer(const llvm::Function &Function);
  void *FunctionPointer(const char *FunctionName);

  void runStaticCtor(llvm::Function *Ctor);
};

}

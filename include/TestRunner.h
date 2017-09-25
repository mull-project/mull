#pragma once

#include "TestResult.h"
#include "MullJIT.h"

#include <llvm/Object/Binary.h>
#include <llvm/Object/ObjectFile.h>
#include <llvm/Target/TargetMachine.h>

namespace mull {

class Test;
class DynamicCallTree;

class TestRunner {
protected:
  TargetMachine &targetMachine;
  MullJIT &jit;
public:
  TestRunner(TargetMachine &machine, MullJIT &jit);

  virtual DynamicCallTree *dynamicCallTree() { return nullptr; }
  virtual std::unique_ptr<CallTree> callTree() { return make_unique<CallTree>(nullptr); };
  virtual void cleanupCallTree(std::unique_ptr<CallTree>) { };
  virtual void prepareForExecution(std::vector<llvm::Module *> &modules) {};

  typedef std::vector<llvm::object::ObjectFile *> ObjectFiles;
  typedef std::vector<llvm::object::OwningBinary<llvm::object::ObjectFile>> OwnedObjectFiles;
  virtual ExecutionResult runTest(Test *Test, ObjectFiles &ObjectFiles) = 0;
  virtual ExecutionResult runTest(Test *test) {
    ExecutionResult result;
    return result;
  }
  virtual ExecutionResult runTest(Test *Test, std::vector<llvm::Module *> &modules) {
    ExecutionResult result;
    return result;
  }

  virtual ~TestRunner() {}
};

}

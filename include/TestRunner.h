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
  virtual std::unique_ptr<RuntimeDyld::SymbolResolver> resolver() = 0;
  virtual std::unique_ptr<SectionMemoryManager> memoryManager() = 0;

  virtual ExecutionResult runTest(Test *test) {
    ExecutionResult result;
    return result;
  }

  virtual ~TestRunner() = default;
};

}

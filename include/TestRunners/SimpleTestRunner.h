#pragma once

#include "llvm/ADT/ArrayRef.h"

namespace llvm {

class ExecutionEngine;
class Function;
class Module;

}

namespace Mutang {

enum TestResult {
  Unused = 0,
  Failed,
  Passed
};

class SimpleTestRunner {
  std::vector<llvm::Module *> Modules;
  llvm::ExecutionEngine *EE;
public:
  SimpleTestRunner(std::vector<llvm::Module *> ModulesRef);
  TestResult runTest(llvm::Function *Test, llvm::Module *TesteeModule);
};

}

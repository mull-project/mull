#pragma once

#include "llvm/ADT/ArrayRef.h"

namespace llvm {

class Module;
class Function;

}

namespace Mutang {

enum TestResult {
  Unused = 0,
  Failed,
  Passed
};

class SimpleTestRunner {
  llvm::ArrayRef<llvm::Module *> Modules;
public:
  SimpleTestRunner(llvm::ArrayRef<llvm::Module *> ModulesRef);
  TestResult runTest(llvm::Function *Test, llvm::Module *TesteeModule);
};

}

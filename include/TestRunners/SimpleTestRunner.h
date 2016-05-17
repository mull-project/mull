#pragma once

#include "llvm/Object/Binary.h"
#include "llvm/Object/ObjectFile.h"

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
public:
  typedef std::vector<llvm::object::OwningBinary<llvm::object::ObjectFile>> ObjectFiles;

  SimpleTestRunner();
  TestResult runTest(llvm::Function *Test, ObjectFiles &ObjectFiles);
};

}

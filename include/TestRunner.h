#pragma once

#include "TestResult.h"

#include "llvm/Object/Binary.h"
#include "llvm/Object/ObjectFile.h"
#include "llvm/Target/TargetMachine.h"

namespace mull {

class Test;

class TestRunner {
protected:
  llvm::TargetMachine &machine;
public:
  typedef std::vector<llvm::object::ObjectFile *> ObjectFiles;
  typedef std::vector<llvm::object::OwningBinary<llvm::object::ObjectFile>> OwnedObjectFiles;
  TestRunner(llvm::TargetMachine &targetMachine);

  virtual ExecutionResult runTest(Test *Test, ObjectFiles &ObjectFiles) = 0;

  virtual ~TestRunner() {}
};

}

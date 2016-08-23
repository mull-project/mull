#pragma once

#include "TestResult.h"

#include "llvm/Object/Binary.h"
#include "llvm/Object/ObjectFile.h"
#include "llvm/Target/TargetMachine.h"

namespace Mutang {

class Test;

class TestRunner {
protected:
  std::unique_ptr<llvm::TargetMachine> TM;
public:
  typedef std::vector<llvm::object::ObjectFile *> ObjectFiles;
  typedef std::vector<llvm::object::OwningBinary<llvm::object::ObjectFile>> OwnedObjectFiles;
  TestRunner();

  virtual ExecutionResult runTest(Test *Test, ObjectFiles &ObjectFiles) = 0;

  virtual ~TestRunner() {}
};

}

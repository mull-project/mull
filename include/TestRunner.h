#pragma once

#include "ExecutionResult.h"

#include <llvm/Object/Binary.h>
#include <llvm/Object/ObjectFile.h>
#include <llvm/Target/TargetMachine.h>

namespace mull {

class Test;

class TestRunner {
protected:
  llvm::TargetMachine &machine;
public:
  typedef std::vector<llvm::object::ObjectFile *> ObjectFiles;
  typedef std::vector<llvm::object::OwningBinary<llvm::object::ObjectFile>> OwnedObjectFiles;
  TestRunner(llvm::TargetMachine &targetMachine);

  virtual void loadProgram(ObjectFiles &objectFiles) = 0;
  virtual ExecutionStatus runTest(Test *test) = 0;

  virtual ~TestRunner() = default;
};

}

#pragma once

#include "LLVMCompatibility.h"
#include "mull/ExecutionResult.h"

#include <llvm/Object/Binary.h>
#include <llvm/Object/ObjectFile.h>

namespace mull {

class JITEngine;
class Test;
class Instrumentation;
class Trampolines;

class TestRunner {
public:
  typedef std::vector<llvm::object::ObjectFile *> ObjectFiles;
  typedef std::vector<llvm::object::OwningBinary<llvm::object::ObjectFile>>
      OwnedObjectFiles;

  TestRunner();

  virtual void loadInstrumentedProgram(ObjectFiles &objectFiles,
                                       Instrumentation &instrumentation,
                                       JITEngine &jit) = 0;
  virtual void loadMutatedProgram(ObjectFiles &objectFiles,
                                  Trampolines &trampolines, JITEngine &jit) = 0;
  virtual ExecutionStatus runTest(Test *test, JITEngine &jit) = 0;

  virtual ~TestRunner() = default;
};

} // namespace mull

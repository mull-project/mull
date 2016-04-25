#include "TestRunners/SimpleTestRunner.h"

using namespace Mutang;
using namespace llvm;

SimpleTestRunner::SimpleTestRunner(ArrayRef<Module *> ModulesRef)
  : Modules(ModulesRef) {}

TestResult SimpleTestRunner::runTest(llvm::Function *Test,
                                     llvm::Module *TesteeModule) {

  return Failed;
}

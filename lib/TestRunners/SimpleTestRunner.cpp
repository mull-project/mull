#include "TestRunners/SimpleTestRunner.h"

#include "llvm/ExecutionEngine/ExecutionEngine.h"
#include "llvm/ExecutionEngine/OrcMCJITReplacement.h"
#include "llvm/ExecutionEngine/GenericValue.h"
#include "llvm/Support/TargetSelect.h"

using namespace Mutang;
using namespace llvm;

SimpleTestRunner::SimpleTestRunner(ArrayRef<Module *> ModulesRef)
  : Modules(ModulesRef) {

    assert(ModulesRef.size() != 0);

    InitializeNativeTarget();
    InitializeNativeTargetAsmPrinter();
    InitializeNativeTargetAsmParser();

    LLVMLinkInMCJIT();
}

TestResult SimpleTestRunner::runTest(Function *Test,
                                     Module *TesteeModule) {

  /// FIXME: Do not create JIT engine every time
  /// If we create the engine once and use
  /// the same module for evaluating a test against original testee
  /// and against mutated testee, then the testee from mutated is not picked up
  /// probably due to some cache
  /// The problem requires investigation

  Module *M = *Modules.begin();

  EngineBuilder builder((std::unique_ptr<Module>(M)));
  builder.setEngineKind(EngineKind::JIT);

  std::string Err;
  builder.setErrorStr(&Err);

  EE = builder.create();

  assert(EE && "Can't create ExecutionEngine");

  for (unsigned long Index = 1; Index < Modules.size(); Index++) {
    Module *M = *(Modules.begin() + Index);
    EE->addModule(std::unique_ptr<Module>(M));
  }

  EE->addModule(std::unique_ptr<Module>(TesteeModule));
  EE->finalizeObject();

  GenericValue GV = EE->runFunction(Test, ArrayRef<GenericValue>());

  EE->removeModule(TesteeModule);

  delete EE;

  const uint64_t *x = GV.IntVal.getRawData();

  if (*x == 1) {
    return Passed;
  }

  return Failed;
}

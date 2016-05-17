#include "TestRunners/SimpleTestRunner.h"

#include "llvm/ExecutionEngine/ExecutionEngine.h"
#include "llvm/ExecutionEngine/OrcMCJITReplacement.h"
#include "llvm/ExecutionEngine/Orc/CompileUtils.h"
#include "llvm/ExecutionEngine/GenericValue.h"
#include "llvm/ExecutionEngine/RTDyldMemoryManager.h"
#include "llvm/ExecutionEngine/SectionMemoryManager.h"
#include "llvm/Support/TargetSelect.h"

using namespace Mutang;
using namespace llvm;

SimpleTestRunner::SimpleTestRunner() {
//
//    assert(ModulesRef.size() != 0);
//
//    InitializeNativeTarget();
//    InitializeNativeTargetAsmPrinter();
//    InitializeNativeTargetAsmParser();

    LLVMLinkInOrcMCJITReplacement();
}

TestResult SimpleTestRunner::runTest(llvm::Function *Test,
                                     ObjectFiles &ObjectFiles) {
  /// Execution Engine requires a Module
  /// Let's give him something
  LLVMContext Ctx;
  Module M("dummy", Ctx);

  EngineBuilder builder((std::unique_ptr<Module>(&M)));
  builder.setEngineKind(EngineKind::JIT);
  builder.setUseOrcMCJITReplacement(true);
  builder.setMCJITMemoryManager(
                                std::unique_ptr<RTDyldMemoryManager>(new SectionMemoryManager()));

  ExecutionEngine *EE = builder.create();

  assert(EE && "Can't create ExecutionEngine");

  for (auto &Obj : ObjectFiles) {
    EE->addObjectFile(std::move(Obj));
  }

  GenericValue GV = EE->runFunction(Test, ArrayRef<GenericValue>());

  const uint64_t *x = GV.IntVal.getRawData();

  if (*x == 1) {
    return Passed;
  }
  
  return Failed;
}

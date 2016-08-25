#include "Driver.h"
#include "ConfigParser.h"
#include "Config.h"
#include "ModuleLoader.h"

#include "SimpleTest/SimpleTestFinder.h"
#include "SimpleTest/SimpleTestRunner.h"

#include "GoogleTest/GoogleTestFinder.h"
#include "GoogleTest/GoogleTestRunner.h"

#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Function.h"
#include "llvm/Support/MemoryBuffer.h"
#include "llvm/Support/SourceMgr.h"
#include "llvm/Support/YAMLParser.h"
#include "llvm/Support/TargetSelect.h"

using namespace Mutang;
using namespace llvm;

static const char *ExecutionResultToString(ExecutionStatus Status) {
  switch (Status) {
    case Failed:
      return "Failed";
    case Passed:
      return "Passed";
    case Invalid:
      llvm_unreachable("Must not reach here");
      break;
  }

  llvm_unreachable("Must not reach here");
}

int main(int argc, char *argv[]) {
  assert(argc == 2);

  ConfigParser Parser;
  auto Cfg = Parser.loadConfig(argv[1]);

  InitializeNativeTarget();
  InitializeNativeTargetAsmPrinter();
  InitializeNativeTargetAsmParser();

  LLVMContext Ctx;
  ModuleLoader Loader(Ctx);

  GoogleTestFinder TestFinder;
  GoogleTestRunner Runner;

  Driver D(*Cfg.get(), Loader, TestFinder, Runner);

  auto Results = D.Run();
  for (auto &R : Results) {

    printf("Result for '%s'\n", R->getTestName().c_str());
    auto TestResult = R->getOriginalTestResult();
    printf("\tOriginal test '%s' in %lld nanoseconds\n", ExecutionResultToString(TestResult.Status), TestResult.RunningTime);
    printf("\tMutants:\n");

    for (auto &MR : R->getMutationResults()) {
      auto MutationResult = MR->getExecutionResult();
      printf("\t\tMutant '%s' in %lld nanoseconds\n", ExecutionResultToString(MutationResult.Status), MutationResult.RunningTime);
    }

  }

  return 0;
}

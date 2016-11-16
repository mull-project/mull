#include "Driver.h"
#include "ConfigParser.h"
#include "Config.h"
#include "ModuleLoader.h"
#include "MutationPoint.h"
#include "SQLiteReporter.h"

#include "Toolchain/Toolchain.h"

#include "SimpleTest/SimpleTestFinder.h"
#include "SimpleTest/SimpleTestRunner.h"

#include "GoogleTest/GoogleTestFinder.h"
#include "GoogleTest/GoogleTestRunner.h"

#include "llvm/IR/LLVMContext.h"
#include "llvm/Support/MemoryBuffer.h"
#include "llvm/Support/SourceMgr.h"
#include "llvm/Support/YAMLParser.h"
#include "llvm/Support/TargetSelect.h"

#include <ctime>
#include <string>

using namespace Mutang;
using namespace llvm;

int debug_main(int argc, char *argv[]) {
  ConfigParser Parser;
  auto config = Parser.loadConfig(argv[argc - 1]);

  LLVMContext Ctx;
  ModuleLoader Loader(Ctx);

  GoogleTestFinder TestFinder;
  Toolchain toolchain(*config.get());
  GoogleTestRunner Runner(toolchain.targetMachine());

  Driver D(*config.get(), Loader, TestFinder, Runner, toolchain);

  if (strcmp(argv[2], "-print-test-names") == 0) {
    D.debug_PrintTestNames();
  } else if (strcmp(argv[2], "-print-testee-names") == 0) {
    D.debug_PrintTesteeNames();
  } else if (strcmp(argv[2], "-print-mutation-points") == 0) {
    D.debug_PrintMutationPoints();
  }

  return 0;
}

int main(int argc, char *argv[]) {
  if (strcmp(argv[1], "-debug") == 0) {
    return debug_main(argc, argv);
  }

  ConfigParser Parser;
  auto config = Parser.loadConfig(argv[1]);

  InitializeNativeTarget();
  InitializeNativeTargetAsmPrinter();
  InitializeNativeTargetAsmParser();

  LLVMContext Ctx;
  ModuleLoader Loader(Ctx);
  Toolchain toolchain(*config.get());

#if 1
  GoogleTestFinder TestFinder;
  GoogleTestRunner Runner(toolchain.targetMachine());
#else
  SimpleTestFinder TestFinder;
  SimpleTestRunner Runner(toolchain.targetMachine());
#endif

  Driver driver(*config.get(), Loader, TestFinder, Runner, toolchain);
  auto results = driver.Run();

  SQLiteReporter reporter;
  reporter.reportResults(results);
  /// It does crash at the very moment
  /// llvm_shutdown();
  return 0;
}

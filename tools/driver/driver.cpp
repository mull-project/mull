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
#include "llvm/Support/CommandLine.h"

#include <ctime>
#include <string>

using namespace Mutang;
using namespace llvm;

cl::OptionCategory MullOptionCategory("Mull");

// Cannot use "debug" as it's already used by LLVM itself.
static cl::opt<bool> Debug(
    "mull-debug",
    llvm::cl::desc("Run in debug mode."),
    llvm::cl::init(false),
    llvm::cl::cat(MullOptionCategory)
);

static cl::opt<bool> PrintTestNames(
    "print-test-names",
    llvm::cl::desc("Print test names."),
    llvm::cl::init(false),
    llvm::cl::cat(MullOptionCategory)
);

static cl::opt<bool> PrintTesteeNames(
    "print-testee-names",
    llvm::cl::desc("Print testee names."),
    llvm::cl::init(false),
    llvm::cl::cat(MullOptionCategory)
);

static cl::opt<bool> PrintMutationPoints(
    "print-mutation-points",
    llvm::cl::desc("Print mutation points."),
    llvm::cl::init(false),
    llvm::cl::cat(MullOptionCategory)
);

static cl::opt<std::string> ConfigFile(
    llvm::cl::desc("<config file>"),
    llvm::cl::Positional
);

int debug_main() {
  ConfigParser Parser;
  auto config = Parser.loadConfig(ConfigFile.c_str());

  LLVMContext Ctx;
  ModuleLoader Loader(Ctx);

  GoogleTestFinder TestFinder;
  Toolchain toolchain(*config.get());
  GoogleTestRunner Runner(toolchain.targetMachine());

  Driver D(*config.get(), Loader, TestFinder, Runner, toolchain);

  if (PrintTestNames) {
    D.debug_PrintTestNames();
  } else if (PrintTesteeNames) {
    D.debug_PrintTesteeNames();
  } else if (PrintMutationPoints) {
    D.debug_PrintMutationPoints();
  }

  return 0;
}

int main(int argc, char *argv[]) {
  // Parse command line options
  cl::HideUnrelatedOptions(MullOptionCategory);
  cl::ParseCommandLineOptions(argc, argv, "Mull");

  if (Debug) {
    return debug_main();
  }

  ConfigParser Parser;
  auto config = Parser.loadConfig(ConfigFile.c_str());

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

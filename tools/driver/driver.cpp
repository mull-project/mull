#include "Driver.h"

#include "Config.h"
#include "ConfigParser.h"
#include "Logger.h"
#include "ModuleLoader.h"
#include "MutationPoint.h"
#include "SQLiteReporter.h"
#include "Result.h"

#include "Toolchain/Toolchain.h"

#include "SimpleTest/SimpleTestFinder.h"
#include "SimpleTest/SimpleTestRunner.h"

#include "GoogleTest/GoogleTestFinder.h"
#include "GoogleTest/GoogleTestRunner.h"

#include "llvm/IR/LLVMContext.h"
#include "llvm/Support/MemoryBuffer.h"
#include "llvm/Support/SourceMgr.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/YAMLParser.h"

#include <ctime>
#include <string>

using namespace mull;
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

  Logger::setLevel(Logger::Level::debug);

  LLVMContext Ctx;
  ModuleLoader Loader(Ctx);
  
  auto mutationOperators = Driver::mutationOperators(config.getMutationOperators());
  
  GoogleTestFinder TestFinder(std::move(mutationOperators));
  Toolchain toolchain(config);
  GoogleTestRunner Runner(toolchain.targetMachine());

  Driver D(config, Loader, TestFinder, Runner, toolchain);

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

  config.dump();

  InitializeNativeTarget();
  InitializeNativeTargetAsmPrinter();
  InitializeNativeTargetAsmParser();

  LLVMContext Ctx;
  ModuleLoader Loader(Ctx);
  Toolchain toolchain(config);

#if 1
  auto mutationOperators = Driver::mutationOperators(config.getMutationOperators());
  GoogleTestFinder TestFinder(std::move(mutationOperators));
  GoogleTestRunner Runner(toolchain.targetMachine());
#else
  SimpleTestFinder TestFinder;
  SimpleTestRunner Runner(toolchain.targetMachine());
#endif

  Driver driver(config, Loader, TestFinder, Runner, toolchain);
  auto result = driver.Run();

  SQLiteReporter reporter(config.getProjectName());
  reporter.reportResults(result);
  llvm_shutdown();
  return EXIT_SUCCESS;
}

#include "Driver.h"

#include "Config.h"
#include "ConfigParser.h"
#include "Logger.h"
#include "ModuleLoader.h"
#include "MutationPoint.h"
#include "SQLiteReporter.h"
#include "Result.h"

#include "Toolchain/Toolchain.h"

#include "GoogleTest/GoogleTestFinder.h"
#include "GoogleTest/GoogleTestRunner.h"
#include "SimpleTest/SimpleTestFinder.h"
#include "SimpleTest/SimpleTestRunner.h"
#include "XCTest/XCTestFinder.h"
#include "XCTest/XCTestRunner.h"

#if defined(MULL_SUPPORT_RUST)
#include "Rust/RustTestFinder.h"
#include "Rust/RustTestRunner.h"
#endif

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

int main(int argc, char *argv[]) {
  if (argc == 1) {
    // TODO: print friendlier help message here.
    Logger::error() << "Usage: mull-driver path-to-config-file.yml" << "\n";
    exit(1);
  }

  // Parse command line options
  cl::HideUnrelatedOptions(MullOptionCategory);
  cl::ParseCommandLineOptions(argc, argv, "Mull");

  ConfigParser Parser;
  auto config = Parser.loadConfig(ConfigFile.c_str());

  config.dump();

  InitializeNativeTarget();
  InitializeNativeTargetAsmPrinter();
  InitializeNativeTargetAsmParser();

  LLVMContext Ctx;
  ModuleLoader Loader(Ctx);
  Toolchain toolchain(config);

  const std::string &testFramework = config.getTestFramework();

  std::unique_ptr<TestFinder> testFinder;
  std::unique_ptr<TestRunner> testRunner;

  auto mutationOperators = Driver::mutationOperators(config.getMutationOperators());

  if (testFramework == "GoogleTest") {
    testFinder = make_unique<GoogleTestFinder>(std::move(mutationOperators),
                                               config.getTests());

    testRunner = make_unique<GoogleTestRunner>(toolchain.targetMachine());
  }

  else if (testFramework == "SimpleTest") {
    testFinder = make_unique<SimpleTestFinder>(std::move(mutationOperators));
    testRunner = make_unique<SimpleTestRunner>(toolchain.targetMachine());
  }

  else if (testFramework == "XCTest") {
    printf("start XCTest\n");
    testFinder = make_unique<XCTestFinder>(std::move(mutationOperators),
                                           config.getTests());
    testRunner = make_unique<XCTestRunner>(toolchain.targetMachine());
  }

  #if defined(MULL_SUPPORT_RUST)
  else if (testFramework == "Rust") {
    testFinder = make_unique<RustTestFinder>(std::move(mutationOperators),
                                             config.getTests());

    testRunner = make_unique<RustTestRunner>(toolchain.targetMachine());
  }
  #endif

  else {
    Logger::error() << "mull-driver> Unknown test framework provided: "
                    << "`" << testFramework << "`. "
                    << "Choose one between: GoogleTest, SimpleTest or Rust "
                    << "(Rust support must be enabled)."
                    << "\n";
    exit(1);
  }

  Driver driver(config, Loader, *testFinder, *testRunner, toolchain);

  if (Debug) {
    if (PrintTestNames) {
      driver.debug_PrintTestNames();
    } else if (PrintTesteeNames) {
      driver.debug_PrintTesteeNames();
    } else if (PrintMutationPoints) {
      driver.debug_PrintMutationPoints();
    }

    return 0;
  }

  auto result = driver.Run();

  SQLiteReporter reporter(config.getProjectName());
  reporter.reportResults(result, config);
  llvm_shutdown();
  return EXIT_SUCCESS;
}

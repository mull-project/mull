#include "Driver.h"

#include "Config.h"
#include "ConfigParser.h"
#include "Filter.h"
#include "Logger.h"
#include "ModuleLoader.h"
#include "MutationOperators/MutationOperatorsFactory.h"
#include "SQLiteReporter.h"
#include "Result.h"
#include "MutationsFinder.h"

#include "Toolchain/Toolchain.h"
#include "Metrics/Metrics.h"

#include "GoogleTest/GoogleTestFinder.h"
#include "GoogleTest/GoogleTestRunner.h"
#include "SimpleTest/SimpleTestFinder.h"
#include "SimpleTest/SimpleTestRunner.h"
#include "CustomTestFramework/CustomTestFinder.h"
#include "CustomTestFramework/CustomTestRunner.h"

#if defined(MULL_SUPPORT_RUST)
#include "Rust/RustTestFinder.h"
#include "Rust/RustTestRunner.h"
#endif

#include <llvm/IR/LLVMContext.h>
#include <llvm/Support/MemoryBuffer.h>
#include <llvm/Support/SourceMgr.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/Support/CommandLine.h>
#include <llvm/Support/YAMLParser.h>

#include <string>

using namespace mull;
using namespace llvm;

cl::OptionCategory MullOptionCategory("Mull");

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

  std::vector <std::string> configErrors = config.validate();
  if (configErrors.size() > 0) {
    Logger::error() << "Provided config file is not valid:" << "\n";
    for (const std::string &configError : configErrors) {
      Logger::error() << "\t" << configError << "\n";
    }

    exit(1);
  }

  config.dump();

  InitializeNativeTarget();
  InitializeNativeTargetAsmPrinter();
  InitializeNativeTargetAsmParser();

  LLVMContext Ctx;
  ModuleLoader Loader(Ctx);
  Toolchain toolchain(config);
  Filter filter;

  for (const std::string &location: config.getExcludeLocations()) {
    filter.skipByLocation(location);
  }

  for (const std::string &test: config.getTests()) {
    filter.includeTest(test);
  }

  const std::string &testFramework = config.getTestFramework();

  std::unique_ptr<TestFinder> testFinder;
  std::unique_ptr<TestRunner> testRunner;

  auto mutationOperatorsFactory = MutationOperatorsFactory();
  auto mutationOperators =
    mutationOperatorsFactory.mutationOperators(config.getMutationOperators());
  MutationsFinder mutationsFinder(std::move(mutationOperators));

  if (testFramework == "GoogleTest") {
    filter.skipByName("testing8internal");
    filter.skipByName("testing15AssertionResult");
    filter.skipByName("testing7Message");
    filter.skipByName("clang_call_terminate");

    filter.skipByLocation("include/c++/v1");
    filter.skipByLocation("gtest");
    filter.skipByLocation("gmock");

    testFinder = make_unique<GoogleTestFinder>();
    testRunner = make_unique<GoogleTestRunner>(toolchain.targetMachine());
  }

  else if (testFramework == "SimpleTest") {
    testFinder = make_unique<SimpleTestFinder>();
    testRunner = make_unique<SimpleTestRunner>(toolchain.targetMachine());
  }

  else if (testFramework == "CustomTest") {
    testFinder = make_unique<CustomTestFinder>(config.getCustomTests());
    testRunner = make_unique<CustomTestRunner>(toolchain.targetMachine());
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

  Metrics metrics;
  Driver driver(config, Loader, *testFinder, *testRunner, toolchain, filter, mutationsFinder, metrics);

  metrics.beginRun();
  auto result = driver.Run();
  metrics.endRun();

  metrics.beginReportResult();
  SQLiteReporter reporter(config.getProjectName());
  reporter.reportResults(result, config, metrics.driverRunTime());
  metrics.endReportResult();

  metrics.dump();

  llvm_shutdown();
  return EXIT_SUCCESS;
}

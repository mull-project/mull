#include "Driver.h"

#include "Config.h"
#include "ConfigParser.h"
#include "Filter.h"
#include "Logger.h"
#include "ModuleLoader.h"
#include "Mutators/MutatorsFactory.h"
#include "Reporters/SQLiteReporter.h"
#include "Reporters/TimeReporter.h"
#include "Result.h"
#include "MutationsFinder.h"

#include "Toolchain/Toolchain.h"
#include "Metrics/Metrics.h"
#include "JunkDetection/JunkDetector.h"
#include "JunkDetection/CXX/CXXJunkDetector.h"

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

  auto mutatorsFactory = MutatorsFactory();
  auto mutators = mutatorsFactory.mutators(config.getMutators());
  MutationsFinder mutationsFinder(std::move(mutators));

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

  std::unique_ptr<JunkDetector> junkDetector(make_unique<NullJunkDetector>());
  if (config.junkDetectionEnabled()) {
    std::string detector = config.junkDetectionConfig().detectorName;
    if (detector == "all") {
      junkDetector = make_unique<AllJunkDetector>();
    } else if (detector == "none") {
      junkDetector = make_unique<NullJunkDetector>();
    } else if (detector == "cxx") {
      junkDetector = make_unique<CXXJunkDetector>(config.junkDetectionConfig());
    } else {
      Logger::error() << "mull-driver> Unknown junk detector provided: "
        << "`" << detector << "`. ";
    }
  }

  std::vector<std::unique_ptr<Reporter>> reporters;
  if (config.getReporters().empty()) {
    reporters.push_back(make_unique<SQLiteReporter>(config.getProjectName()));
  }

  for (auto &reporter: config.getReporters()) {
    if (reporter == "sqlite") {
      reporters.push_back(make_unique<SQLiteReporter>(config.getProjectName()));
    }

    else if (reporter == "time") {
      reporters.push_back(make_unique<TimeReporter>());
    }

    else {
      Logger::error() << "mull-driver> Unknown reporter provided: "
        << "`" << reporter << "`. ";
    }
  }

  Metrics metrics;
  Driver driver(config, Loader, *testFinder, *testRunner, toolchain, filter, mutationsFinder, metrics, *junkDetector);

  metrics.beginRun();
  auto result = driver.Run();
  metrics.endRun();

  for (auto &reporter: reporters) {
    reporter->reportResults(*result, config, metrics);
  }

  llvm_shutdown();
  return EXIT_SUCCESS;
}

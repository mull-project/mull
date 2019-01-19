#include "Driver.h"

#include "Config/RawConfig.h"
#include "Config/ConfigParser.h"
#include "Config/Configuration.h"
#include "Filter.h"
#include "Logger.h"
#include "ModuleLoader.h"
#include "Mutators/MutatorsFactory.h"
#include "Reporters/SQLiteReporter.h"
#include "Reporters/TimeReporter.h"
#include "Result.h"
#include "MutationsFinder.h"
#include "Parallelization/TaskExecutor.h"
#include "Parallelization/Tasks/LoadObjectFilesTask.h"
#include "Toolchain/Toolchain.h"
#include "Metrics/Metrics.h"
#include "JunkDetection/JunkDetector.h"
#include "JunkDetection/CXX/CXXJunkDetector.h"
#include "TestFrameworks/TestFrameworkFactory.h"
#include "Program/Program.h"

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

  MetricsMeasure totalExecutionTime;
  totalExecutionTime.start();

  // Parse command line options
  cl::HideUnrelatedOptions(MullOptionCategory);
  cl::ParseCommandLineOptions(argc, argv, "Mull");

  ConfigParser Parser;
  auto rawConfig = Parser.loadConfig(ConfigFile.c_str());

  std::vector <std::string> configErrors = rawConfig.validate();
  if (configErrors.size() > 0) {
    Logger::error() << "Provided config file is not valid:" << "\n";
    for (const std::string &configError : configErrors) {
      Logger::error() << "\t" << configError << "\n";
    }

    exit(1);
  }

  rawConfig.dump();
  Configuration configuration(rawConfig);

  InitializeNativeTarget();
  InitializeNativeTargetAsmPrinter();
  InitializeNativeTargetAsmParser();

  Toolchain toolchain(configuration);
  Filter filter;

  for (const std::string &location: rawConfig.getExcludeLocations()) {
    filter.skipByLocation(location);
  }

  for (const std::string &test: rawConfig.getTests()) {
    filter.includeTest(test);
  }

  const std::string &testFrameworkName = rawConfig.getTestFramework();

  auto mutatorsFactory = MutatorsFactory();
  auto mutators = mutatorsFactory.mutators(rawConfig.getMutators());
  MutationsFinder mutationsFinder(std::move(mutators), configuration);

  if (testFrameworkName == "GoogleTest") {
    filter.skipByName("testing8internal");
    filter.skipByName("testing15AssertionResult");
    filter.skipByName("testing7Message");
    filter.skipByName("clang_call_terminate");

    filter.skipByLocation("include/c++/v1");
    filter.skipByLocation("gtest");
    filter.skipByLocation("gmock");
  }

  TestFrameworkFactory testFrameworkFactory(configuration, toolchain);
  TestFramework testFramework(testFrameworkFactory.createTestFramework(testFrameworkName));

  std::unique_ptr<JunkDetector> junkDetector(make_unique<NullJunkDetector>());
  if (configuration.junkDetectionEnabled) {
    std::string detector = rawConfig.junkDetectionConfig().detectorName;
    if (detector == "all") {
      junkDetector = make_unique<AllJunkDetector>();
    } else if (detector == "none") {
      junkDetector = make_unique<NullJunkDetector>();
    } else if (detector == "cxx") {
      junkDetector = make_unique<CXXJunkDetector>(rawConfig.junkDetectionConfig());
    } else {
      Logger::error() << "mull-driver> Unknown junk detector provided: "
        << "`" << detector << "`. ";
    }
  }

  std::vector<std::unique_ptr<Reporter>> reporters;
  if (rawConfig.getReporters().empty()) {
    reporters.push_back(make_unique<SQLiteReporter>(rawConfig.getProjectName()));
  }

  for (auto &reporter: rawConfig.getReporters()) {
    if (reporter == "sqlite") {
      reporters.push_back(make_unique<SQLiteReporter>(rawConfig.getProjectName()));
    }

    else if (reporter == "time") {
      reporters.push_back(make_unique<TimeReporter>());
    }

    else {
      Logger::error() << "mull-driver> Unknown reporter provided: "
        << "`" << reporter << "`. ";
    }
  }

  ObjectFiles objectFiles;
  std::vector<LoadObjectFilesTask> tasks;
  for (int i = 0; i < configuration.parallelization.workers; i++) {
    tasks.emplace_back(LoadObjectFilesTask());
  }
  TaskExecutor<LoadObjectFilesTask> objectsLoader(
      "Loading precompiled object files", configuration.objectFilePaths,
      objectFiles, tasks);
  objectsLoader.execute();

  ModuleLoader moduleLoader;
  Program program(configuration.dynamicLibraryPaths, std::move(objectFiles),
                  moduleLoader.loadModules(configuration));

  Metrics metrics;
  Driver driver(configuration, program, testFramework, toolchain, filter, mutationsFinder, metrics, *junkDetector);

  metrics.beginRun();
  auto result = driver.Run();
  metrics.endRun();

  for (auto &reporter: reporters) {
    reporter->reportResults(*result, rawConfig, metrics);
  }

  llvm_shutdown();

  totalExecutionTime.finish();
  Logger::info() << "\nTotal execution time: " << totalExecutionTime.duration()
                 << MetricsMeasure::precision() << "\n";

  return EXIT_SUCCESS;
}

#include "mull/Driver.h"

#include "mull/BitcodeLoader.h"
#include "mull/Config/ConfigParser.h"
#include "mull/Config/Configuration.h"
#include "mull/Config/RawConfig.h"
#include "mull/Filter.h"
#include "mull/JunkDetection/CXX/CXXJunkDetector.h"
#include "mull/JunkDetection/JunkDetector.h"
#include "mull/Logger.h"
#include "mull/Metrics/Metrics.h"
#include "mull/MutationFilters/JunkMutationFilter.h"
#include "mull/MutationsFinder.h"
#include "mull/Mutators/MutatorsFactory.h"
#include "mull/Parallelization/TaskExecutor.h"
#include "mull/Parallelization/Tasks/LoadObjectFilesTask.h"
#include "mull/Program/Program.h"
#include "mull/Reporters/SQLiteReporter.h"
#include "mull/Reporters/TimeReporter.h"
#include "mull/Result.h"
#include "mull/TestFrameworks/TestFrameworkFactory.h"
#include "mull/Toolchain/Toolchain.h"
#include "mull/Version.h"

#include <llvm/IR/LLVMContext.h>
#include <llvm/Support/CommandLine.h>
#include <llvm/Support/MemoryBuffer.h>
#include <llvm/Support/SourceMgr.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/Support/YAMLParser.h>

#include <string>

using namespace mull;
using namespace llvm;

cl::OptionCategory MullOptionCategory("Mull");

static cl::opt<std::string> ConfigFile(llvm::cl::desc("<config file>"),
                                       llvm::cl::Positional);

int main(int argc, char *argv[]) {
  if (argc == 1) {
    // TODO: print friendlier help message here.
    Logger::error() << "Usage: mull-driver path-to-config-file.yml"
                    << "\n";
    exit(1);
  }

  llvm_compat::setVersionPrinter(printVersionInformation,
                                 printVersionInformationStream);

  MetricsMeasure totalExecutionTime;
  totalExecutionTime.start();

  // Parse command line options
  cl::HideUnrelatedOptions(MullOptionCategory);
  cl::ParseCommandLineOptions(argc, argv, "Mull");

  ConfigParser Parser;
  auto rawConfig = Parser.loadConfig(ConfigFile.c_str());

  std::vector<std::string> configErrors = rawConfig.validate();
  if (configErrors.size() > 0) {
    Logger::error() << "Provided config file is not valid:"
                    << "\n";
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

  for (const std::string &location : rawConfig.getExcludeLocations()) {
    filter.skipByLocation(location);
  }

  for (const std::string &test : rawConfig.getTests()) {
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

  TestFrameworkFactory testFrameworkFactory;
  TestFramework testFramework(testFrameworkFactory.createTestFramework(
      testFrameworkName, toolchain, configuration));

  std::unique_ptr<JunkDetector> junkDetector(make_unique<NullJunkDetector>());
  if (rawConfig.junkDetectionEnabled()) {
    std::string detector = rawConfig.junkDetectionConfig().detectorName;
    if (detector == "all") {
      junkDetector = make_unique<AllJunkDetector>();
    } else if (detector == "none") {
      junkDetector = make_unique<NullJunkDetector>();
    } else if (detector == "cxx") {
      mull::JunkDetectionConfig junkDetectionConfig;
      mull::ASTStorage astStorage(
          junkDetectionConfig.cxxCompilationDatabasePath,
          junkDetectionConfig.cxxCompilationFlags);
      junkDetector = make_unique<mull::CXXJunkDetector>(astStorage);
    } else {
      Logger::error() << "mull-driver> Unknown junk detector provided: "
                      << "`" << detector << "`. ";
    }
  }
  JunkMutationFilter junkMutationFilter(*junkDetector.get());

  std::vector<std::unique_ptr<Reporter>> reporters;
  if (rawConfig.getReporters().empty()) {
    reporters.push_back(
        make_unique<SQLiteReporter>(rawConfig.getProjectName()));
  }

  for (auto &reporter : rawConfig.getReporters()) {
    if (reporter == "sqlite") {
      reporters.push_back(
          make_unique<SQLiteReporter>(rawConfig.getProjectName()));
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

  BitcodeLoader bitcodeLoader;
  Program program(configuration.dynamicLibraryPaths, std::move(objectFiles),
                  bitcodeLoader.loadBitcode(configuration));

  std::unique_ptr<ProcessSandbox> sandbox(nullptr);
  if (rawConfig.forkEnabled()) {
    sandbox = llvm::make_unique<ForkTimerSandbox>();
  } else {
    sandbox = llvm::make_unique<NullProcessSandbox>();
  }

  std::vector<MutationFilter *> mutationFilters;
  mutationFilters.push_back(&junkMutationFilter);

  Metrics metrics;
  Driver driver(configuration, *sandbox, program, toolchain, mutationFilters,
                filter, mutationsFinder, metrics, testFramework);

  metrics.beginRun();
  auto result = driver.Run();
  metrics.endRun();

  for (auto &reporter : reporters) {
    reporter->reportResults(*result, rawConfig, metrics);
  }

  llvm_shutdown();

  totalExecutionTime.finish();
  Logger::info() << "\nTotal execution time: " << totalExecutionTime.duration()
                 << MetricsMeasure::precision() << "\n";

  return EXIT_SUCCESS;
}

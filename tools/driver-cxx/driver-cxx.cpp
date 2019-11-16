#include <ebc/BitcodeContainer.h>
#include <ebc/BitcodeRetriever.h>
#include <ebc/EmbeddedFile.h>

#include <llvm/Support/TargetSelect.h>

#include <unistd.h>

#include "DynamicLibraries.h"
#include "mull/Parallelization/Tasks/LoadBitcodeFromBinaryTask.h"
#include "mull/Config/Configuration.h"
#include "mull/Config/ConfigurationOptions.h"
#include "mull/Driver.h"
#include "mull/Filters/FilePathFilter.h"
#include "mull/Filters/Filters.h"
#include "mull/Filters/JunkMutationFilter.h"
#include "mull/Filters/NoDebugInfoFilter.h"
#include "mull/JunkDetection/CXX/CXXJunkDetector.h"
#include "mull/Metrics/Metrics.h"
#include "mull/MutationsFinder.h"
#include "mull/Mutators/MutatorsFactory.h"
#include "mull/Parallelization/Parallelization.h"
#include "mull/Program/Program.h"
#include "mull/Reporters/ASTSourceInfoProvider.h"
#include "mull/Reporters/IDEReporter.h"
#include "mull/Reporters/MutationTestingElementsReporter.h"
#include "mull/Reporters/SQLiteReporter.h"
#include "mull/Result.h"
#include "mull/Sandbox/ProcessSandbox.h"
#include "mull/TestFrameworks/TestFrameworkFactory.h"
#include "mull/Version.h"

llvm::cl::OptionCategory MullCXXCategory("mull-cxx");

llvm::cl::opt<std::string> InputFile(llvm::cl::Positional,
                                     llvm::cl::desc("<input file>"),
                                     llvm::cl::Required);

llvm::cl::opt<unsigned> Workers("workers", llvm::cl::Optional,
                                llvm::cl::desc("How many threads to use"),
                                llvm::cl::cat(MullCXXCategory));

llvm::cl::opt<std::string> CompilationDatabasePath(
    "compdb-path", llvm::cl::Optional,
    llvm::cl::desc("Path to a compilation database (compile_commands.json) for "
                   "junk detection"),
    llvm::cl::cat(MullCXXCategory), llvm::cl::init(""));

llvm::cl::opt<std::string> CompilationFlags(
    "compilation-flags", llvm::cl::Optional,
    llvm::cl::desc("Extra compilation flags for junk detection"),
    llvm::cl::cat(MullCXXCategory), llvm::cl::init(""));

llvm::cl::opt<std::string> CacheDir(
    "cache-dir", llvm::cl::Optional,
    llvm::cl::desc("Where to store cache (defaults to /tmp/mull-cache)"),
    llvm::cl::cat(MullCXXCategory), llvm::cl::init("/tmp/mull-cache"));

llvm::cl::opt<std::string>
    ReportDirectory("report-dir", llvm::cl::Optional,
                    llvm::cl::desc("Where to store report (defaults to '.')"),
                    llvm::cl::cat(MullCXXCategory), llvm::cl::init("."));

llvm::cl::opt<std::string> ReportName(
    "report-name", llvm::cl::Optional,
    llvm::cl::desc("Filename for the report (only for supported reporters). "
                   "Defaults to <timestamp>.<extension>"),
    llvm::cl::cat(MullCXXCategory), llvm::cl::init("."));

llvm::cl::opt<bool>
    DisableCache("disable-cache", llvm::cl::Optional,
                 llvm::cl::desc("Disables cache (enabled by default)"),
                 llvm::cl::cat(MullCXXCategory), llvm::cl::init(false));

llvm::cl::opt<bool>
    DryRun("dry-run", llvm::cl::Optional,
                 llvm::cl::desc("Skips real mutants execution"),
                 llvm::cl::cat(MullCXXCategory), llvm::cl::init(false));

enum MutatorsOptionIndex : int { _mutatorsOptionIndex_unused };
llvm::cl::list<MutatorsOptionIndex> Mutators("mutators", llvm::cl::ZeroOrMore,
                                             llvm::cl::desc("Choose mutators:"),
                                             llvm::cl::cat(MullCXXCategory));

class MutatorsCLIOptions {
public:
  explicit MutatorsCLIOptions(llvm::cl::list<MutatorsOptionIndex> &parameter)
      : options(factory.commandLineOptions()), parameter(parameter) {
    int index = 0;
    for (auto &option : options) {
      parameter.getParser().addLiteralOption(option.first.c_str(), index++,
                                             option.second.c_str());
    }
  }

  std::vector<std::unique_ptr<mull::Mutator>> mutators() {
    std::vector<std::string> selectedGroups;
    for (int i = 0; i < parameter.size(); i++) {
      auto &name = parameter[i];
      selectedGroups.push_back(options[name].first);
    }

    return factory.mutators(selectedGroups);
  }

private:
  mull::MutatorsFactory factory;
  std::vector<std::pair<std::string, std::string>> options;
  llvm::cl::list<MutatorsOptionIndex> &parameter;
};

enum TestFrameworkOptionIndex : int { _testFrameworkOptionIndex_unused };
llvm::cl::opt<TestFrameworkOptionIndex>
    TestFrameworks("test-framework", llvm::cl::Required,
                   llvm::cl::desc("Choose test framework:"),
                   llvm::cl::cat(MullCXXCategory));

class TestFrameworkCLIOptions {
public:
  explicit TestFrameworkCLIOptions(
      llvm::cl::opt<TestFrameworkOptionIndex> &parameter)
      : options(factory.commandLineOptions()), parameter(parameter) {
    int index = 0;
    for (auto &option : options) {
      parameter.getParser().addLiteralOption(option.first.c_str(), index++,
                                             option.second.c_str());
    }
  }

  mull::TestFramework testFramework(mull::Toolchain &toolchain,
                                    mull::Configuration &configuration) {
    auto &name = options[parameter.getValue()].first;
    return factory.createTestFramework(name, toolchain, configuration);
  }

private:
  mull::TestFrameworkFactory factory;
  std::vector<std::pair<std::string, std::string>> options;
  llvm::cl::opt<TestFrameworkOptionIndex> &parameter;
};

llvm::cl::list<std::string> LDSearchPaths("ld_search_path",
                                          llvm::cl::ZeroOrMore,
                                          llvm::cl::desc("Library search path"),
                                          llvm::cl::cat(MullCXXCategory));

llvm::cl::list<std::string> ExcludePaths(
    "exclude-path", llvm::cl::ZeroOrMore,
    llvm::cl::desc("File/directory paths to ignore (supports regex)"),
    llvm::cl::cat(MullCXXCategory));

llvm::cl::list<std::string> IncludePaths(
  "include-path", llvm::cl::ZeroOrMore,
  llvm::cl::desc("File/directory paths to whitelist (supports regex)"),
  llvm::cl::cat(MullCXXCategory));

#if LLVM_VERSION_MAJOR == 3
#define TRAILING_NULL , nullptr
#else
#define TRAILING_NULL
#endif

enum SandboxType { None, Watchdog, Timer };
llvm::cl::opt<SandboxType> SandboxOption(
    "sandbox", llvm::cl::desc("Choose sandbox level:"),
    llvm::cl::values(clEnumVal(None, "No sandboxing"),
                     clEnumVal(Watchdog, "Uses 4 processes, not recommended"),
                     clEnumVal(Timer, "Fastest, Recommended") TRAILING_NULL),
    llvm::cl::cat(MullCXXCategory), llvm::cl::init(Timer));

enum ReporterType { IDE, SQLite, Elements };
llvm::cl::list<ReporterType> ReporterOption(
    "reporters", llvm::cl::ZeroOrMore, llvm::cl::desc("Choose reporters:"),
    llvm::cl::values(
        clEnumVal(IDE, "Prints compiler-like warnings into stdout"),
        clEnumVal(SQLite, "Saves results into an SQLite database"),
        clEnumVal(Elements,
                  "Generates mutation-testing-elements compatible JSON file")
            TRAILING_NULL),
    llvm::cl::cat(MullCXXCategory));

std::unique_ptr<mull::ProcessSandbox>
GetProcessSandbox(llvm::cl::opt<SandboxType> &option) {
  if (option == SandboxType::None) {
    return llvm::make_unique<mull::NullProcessSandbox>();
  }
  if (option == SandboxType::Watchdog) {
    return llvm::make_unique<mull::ForkWatchdogSandbox>();
  }

  return llvm::make_unique<mull::ForkTimerSandbox>();
}

static void validateInputFile() {
  if (access(InputFile.getValue().c_str(), R_OK) != 0) {
    perror(InputFile.getValue().c_str());
    exit(1);
  }
}

int main(int argc, char **argv) {
  llvm_compat::setVersionPrinter(mull::printVersionInformation,
                                 mull::printVersionInformationStream);
  MutatorsCLIOptions mutatorsOptions(Mutators);
  TestFrameworkCLIOptions testFrameworkOption(TestFrameworks);

  llvm::cl::HideUnrelatedOptions(MullCXXCategory);
  llvm::cl::ParseCommandLineOptions(argc, argv);

  validateInputFile();

  mull::MetricsMeasure totalExecutionTime;
  totalExecutionTime.start();

  llvm::InitializeNativeTarget();
  llvm::InitializeNativeTargetAsmPrinter();
  llvm::InitializeNativeTargetAsmParser();

  mull::Configuration configuration;
  configuration.dryRunEnabled = DryRun.getValue();
  configuration.customTests.push_back(
      mull::CustomTestDefinition("main", "main", "mull", {}));
  configuration.customTests.push_back(
      mull::CustomTestDefinition("main", "_main", "mull", {}));
  configuration.failFastEnabled = true;

  if (Workers) {
    mull::ParallelizationConfig parallelizationConfig;
    parallelizationConfig.workers = Workers;
    parallelizationConfig.normalize();
    configuration.parallelization = parallelizationConfig;
  } else {
    configuration.parallelization =
        mull::ParallelizationConfig::defaultConfig();
  }

  if (!DisableCache.getValue()) {
    configuration.cacheEnabled = true;
    configuration.cacheDirectory = CacheDir.getValue();
  }

  std::vector<std::unique_ptr<ebc::EmbeddedFile>> embeddedFiles;
  mull::SingleTaskExecutor extractBitcodeBuffers(
      "Extracting bitcode from executable", [&] {
        ebc::BitcodeRetriever bitcodeRetriever(InputFile.getValue());
        for (auto &bitcodeInfo : bitcodeRetriever.GetBitcodeInfo()) {
          auto &container = bitcodeInfo.bitcodeContainer;
          if (container) {
            for (auto &file : container->GetRawEmbeddedFiles()) {
              embeddedFiles.push_back(std::move(file));
            }
          } else {
            mull::Logger::warn() << "No bitcode: " << bitcodeInfo.arch << "\n";
          }
        }
      });
  extractBitcodeBuffers.execute();

  std::vector<std::unique_ptr<llvm::LLVMContext>> contexts;
  std::vector<mull::LoadBitcodeFromBinaryTask> tasks;
  for (int i = 0; i < configuration.parallelization.workers; i++) {
    auto context = llvm::make_unique<llvm::LLVMContext>();
    tasks.emplace_back(mull::LoadBitcodeFromBinaryTask(*context));
    contexts.push_back(std::move(context));
  }
  std::vector<std::unique_ptr<mull::Bitcode>> bitcode;
  mull::TaskExecutor<mull::LoadBitcodeFromBinaryTask> executor(
      "Loading bitcode files", embeddedFiles, bitcode, std::move(tasks));
  executor.execute();

  std::vector<std::string> librarySearchPaths;
  for (auto &searchPath : LDSearchPaths) {
    librarySearchPaths.push_back(searchPath);
  }

  auto dynamicLibraries =
      mull::findDynamicLibraries(InputFile.getValue(), librarySearchPaths);

  mull::Program program(dynamicLibraries, {}, std::move(bitcode));

  mull::Toolchain toolchain(configuration);

  mull::TestFramework testFramework(
      testFrameworkOption.testFramework(toolchain, configuration));

  bool junkDetectionEnabled = false;
  std::string cxxCompilationFlags;
  std::string cxxCompilationDatabasePath;
  if (!CompilationFlags.empty()) {
    cxxCompilationFlags = CompilationFlags.getValue();
    junkDetectionEnabled = true;
  }
  if (!CompilationDatabasePath.empty()) {
    cxxCompilationDatabasePath = CompilationDatabasePath.getValue();
    junkDetectionEnabled = true;
  }
  mull::ASTStorage astStorage(cxxCompilationDatabasePath, cxxCompilationFlags);
  mull::ASTSourceInfoProvider sourceInfoProvider(astStorage);
  mull::CXXJunkDetector junkDetector(astStorage);
  mull::JunkMutationFilter junkFilter(junkDetector);

  mull::MutationsFinder mutationsFinder(mutatorsOptions.mutators(),
                                        configuration);

  mull::Metrics metrics;

  auto sandbox = GetProcessSandbox(SandboxOption);

  mull::Filters filters;

  mull::NoDebugInfoFilter noDebugInfoFilter;
  filters.mutationFilters.push_back(&noDebugInfoFilter);
  filters.functionFilters.push_back(&noDebugInfoFilter);
  filters.instructionFilters.push_back(&noDebugInfoFilter);

  mull::FilePathFilter filePathFilter;
  filters.mutationFilters.push_back(&filePathFilter);
  filters.functionFilters.push_back(&filePathFilter);

  for (const auto &regex : ExcludePaths) {
    filePathFilter.exclude(regex);
  }
  for (const auto &regex : IncludePaths) {
    filePathFilter.include(regex);
  }

  if (junkDetectionEnabled) {
    filters.mutationFilters.push_back(&junkFilter);
  }

  mull::Driver driver(configuration, *sandbox, program, toolchain, filters,
                      mutationsFinder, metrics, testFramework);
  metrics.beginRun();
  auto result = driver.Run();
  metrics.endRun();

  std::vector<std::unique_ptr<mull::Reporter>> reporters;
  for (auto i = 0; i < ReporterOption.getNumOccurrences(); i++) {
    switch (ReporterOption[i]) {
    case IDE: {
      reporters.push_back(llvm::make_unique<mull::IDEReporter>());
    } break;
    case SQLite: {
      reporters.push_back(
          llvm::make_unique<mull::SQLiteReporter>(ReportDirectory, ReportName));
    } break;
    case Elements: {
      if (junkDetectionEnabled) {
        reporters.push_back(
            llvm::make_unique<mull::MutationTestingElementsReporter>(
                ReportDirectory, ReportName, sourceInfoProvider));
      } else {
        mull::Logger::warn()
            << "The Mutation Testing Elements Reporter requires "
               "the compilation database to be provided";
      }
    } break;
    }
  }
  if (reporters.empty()) {
    reporters.push_back(llvm::make_unique<mull::IDEReporter>());
  }

  for (auto &reporter : reporters) {
    reporter->reportResults(*result, metrics);
  }

  llvm::llvm_shutdown();

  totalExecutionTime.finish();
  mull::Logger::info() << "\nTotal execution time: "
                       << totalExecutionTime.duration()
                       << mull::MetricsMeasure::precision() << "\n";

  return 0;
}

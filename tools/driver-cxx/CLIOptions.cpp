#include "CLIOptions.h"

using namespace mull;
using namespace tool;
using namespace llvm::cl;

OptionCategory tool::MullCXXCategory("mull-cxx");

// clang-format off

opt<std::string> tool::InputFile(
  Positional,
  desc("<input file>"),
  Required,
  cat(MullCXXCategory));

opt<unsigned> tool::Workers(
  "workers",
  desc("How many threads to use"),
  Optional,
  cat(MullCXXCategory));

opt<std::string> tool::ReportDirectory(
  "report-dir",
  desc("Where to store report (defaults to '.')"),
  Optional,
  init("."),
  cat(MullCXXCategory));

opt<std::string> tool::ReportName(
  "report-name",
  desc("Filename for the report (only for supported reporters). Defaults to <timestamp>.<extension>"),
  Optional,
  init("."),
  cat(MullCXXCategory));

opt<bool> tool::DisableCache(
  "disable-cache",
  desc("Disables cache (enabled by default)"),
  Optional,
  init(false),
  cat(MullCXXCategory));

opt<std::string> tool::CacheDir(
  "cache-dir",
  desc("Where to store cache (defaults to /tmp/mull-cache)"),
  Optional,
  init("/tmp/mull-cache"),
  cat(MullCXXCategory));

opt<std::string> tool::CompilationDatabasePath(
  "compdb-path",
  desc("Path to a compilation database (compile_commands.json) for junk detection"),
  Optional,
  cat(MullCXXCategory));

opt<std::string> tool::CompilationFlags(
  "compilation-flags",
  desc("Extra compilation flags for junk detection"),
  Optional,
  cat(MullCXXCategory));

opt<bool> tool::DryRunOption(
  "dry-run",
  desc("Skips real mutants execution"),
  Optional,
  init(false),
  cat(MullCXXCategory));

list<MutatorsOptionIndex> tool::Mutators(
  "mutators",
  desc("Choose mutators:"),
  ZeroOrMore,
  cat(MullCXXCategory));

opt<TestFrameworkOptionIndex> tool::TestFrameworks(
  "test-framework",
  desc("Choose test framework:"),
  Required,
  cat(MullCXXCategory));

list<std::string> tool::LDSearchPaths(
  "ld-search-path",
  desc("Library search path"),
  ZeroOrMore,
  cat(MullCXXCategory));

list<std::string> tool::ExcludePaths(
  "exclude-path",
  desc("File/directory paths to ignore (supports regex)"),
  ZeroOrMore,
  cat(MullCXXCategory));

list<std::string> tool::IncludePaths(
  "include-path",
  desc("File/directory paths to whitelist (supports regex)"),
  ZeroOrMore,
  cat(MullCXXCategory));

#if LLVM_VERSION_MAJOR == 3
#define TRAILING_NULL , nullptr
#else
#define TRAILING_NULL
#endif

opt<SandboxType> tool::SandboxOption(
  "sandbox",
  desc("Choose sandbox level:"),
  values(clEnumVal(None, "No sandboxing"),
         clEnumVal(Watchdog, "Uses 4 processes, not recommended"),
         clEnumVal(Timer, "Fastest, Recommended")
         TRAILING_NULL),
  init(Timer),
  cat(MullCXXCategory));


list<ReporterType> tool::ReporterOption(
  "reporters",
  desc("Choose reporters:"),
  OneOrMore,
  values(clEnumVal(IDE, "Prints compiler-like warnings into stdout"),
         clEnumVal(SQLite, "Saves results into an SQLite database"),
         clEnumVal(Elements, "Generates mutation-testing-elements compatible JSON file")
         TRAILING_NULL),
  cat(MullCXXCategory));

// clang-format on

MutatorsCLIOptions::MutatorsCLIOptions(list<MutatorsOptionIndex> &parameter)
    : options(factory.commandLineOptions()), parameter(parameter) {
  int index = 0;
  for (auto &option : options) {
    parameter.getParser().addLiteralOption(option.first.c_str(), index++, option.second.c_str());
  }
}

std::vector<std::unique_ptr<Mutator>> MutatorsCLIOptions::mutators() {
  std::vector<std::string> selectedGroups;
  for (int i = 0; i < parameter.size(); i++) {
    auto &name = parameter[i];
    selectedGroups.push_back(options[name].first);
  }

  return factory.mutators(selectedGroups);
}

TestFrameworkCLIOptions::TestFrameworkCLIOptions(opt<TestFrameworkOptionIndex> &parameter)
    : options(factory.commandLineOptions()), parameter(parameter) {
  int index = 0;
  for (auto &option : options) {
    parameter.getParser().addLiteralOption(option.first.c_str(), index++, option.second.c_str());
  }
}

TestFramework TestFrameworkCLIOptions::testFramework(Toolchain &toolchain,
                                                     Configuration &configuration) {
  auto &name = options[parameter.getValue()].first;
  return factory.createTestFramework(name, toolchain, configuration);
}

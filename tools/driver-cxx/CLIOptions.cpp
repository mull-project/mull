#include "CLIOptions.h"
#include <mull/Reporters/IDEReporter.h>
#include <mull/Reporters/MutationTestingElementsReporter.h>
#include <mull/Reporters/SQLiteReporter.h>

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
  init(""),
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

opt<SandboxKind> tool::SandboxOption(
    "sandbox",
    desc("Choose sandbox approach:"),
    Optional,
    init(SandboxKind::Timer),
    cat(MullCXXCategory));

list<ReporterKind> tool::ReportersOption(
  "reporters",
  desc("Choose reporters:"),
  ZeroOrMore,
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

SandboxCLIOptions::SandboxCLIOptions(opt<SandboxKind> &parameter) : parameter(parameter) {
  parameter.getParser().addLiteralOption("None", SandboxKind::NoSandbox, "No sandboxing");
  parameter.getParser().addLiteralOption(
      "Watchdog", SandboxKind::Watchdog, "Uses 4 processes, not recommended");
  parameter.getParser().addLiteralOption("Timer", SandboxKind::Timer, "Fastest, Recommended");
}

std::unique_ptr<mull::ProcessSandbox> SandboxCLIOptions::sandbox() {
  switch (parameter.getValue()) {
  case SandboxKind::NoSandbox:
    return llvm::make_unique<mull::NullProcessSandbox>();
  case SandboxKind::Watchdog:
    return llvm::make_unique<mull::ForkWatchdogSandbox>();
  case SandboxKind::Timer:
    return llvm::make_unique<mull::ForkTimerSandbox>();
  }
}

ReportersCLIOptions::ReportersCLIOptions(list<ReporterKind> &parameter) : parameter(parameter) {
  parameter.getParser().addLiteralOption(
      "IDE", ReporterKind::IDE, "Prints compiler-like warnings into stdout");
  parameter.getParser().addLiteralOption(
      "SQLite", ReporterKind::SQLite, "Saves results into an SQLite database");
  parameter.getParser().addLiteralOption(
      "Elements",
      ReporterKind::Elements,
      "Generates mutation-testing-elements compatible JSON file");
}

std::vector<std::unique_ptr<Reporter>> ReportersCLIOptions::reporters(ReporterParameters params) {
  std::vector<std::unique_ptr<mull::Reporter>> reporters;
  std::string &name = params.reporterName;
  std::string &directory = params.reporterDirectory;
  SourceInfoProvider &provider = params.sourceInfoProvider;

  // TODO: Move to a better place
  for (auto i = 0; i < parameter.getNumOccurrences(); i++) {
    switch (parameter[i]) {
    case ReporterKind::IDE: {
      reporters.push_back(llvm::make_unique<mull::IDEReporter>());
    } break;
    case ReporterKind::SQLite: {
      reporters.emplace_back(new mull::SQLiteReporter(directory, name));
    } break;
    case ReporterKind::Elements: {
      reporters.emplace_back(new mull::MutationTestingElementsReporter(directory, name, provider));
    } break;
    }
  }

  if (reporters.empty()) {
    reporters.push_back(llvm::make_unique<mull::IDEReporter>());
  }

  return reporters;
}

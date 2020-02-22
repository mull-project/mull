#include "CLIOptions.h"
#include <mull/Mutators/Mutator.h>
#include <mull/Reporters/IDEReporter.h>
#include <mull/Reporters/MutationTestingElementsReporter.h>
#include <mull/Reporters/SQLiteReporter.h>
#include <sstream>

using namespace mull;
using namespace tool;
using namespace llvm::cl;

OptionCategory tool::MullCXXCategory("mull-cxx");

// clang-format off

opt<std::string> tool::InputFile(
  Positional,
  desc("<input file>"),
  Required,
  value_desc("path"),
  cat(MullCXXCategory));

opt<unsigned> tool::Workers(
  "workers",
  desc("How many threads to use"),
  Optional,
  value_desc("number"),
  cat(MullCXXCategory));

opt<std::string> tool::ReportDirectory(
  "report-dir",
  desc("Where to store report (defaults to '.')"),
  Optional,
  value_desc("directory"),
  init("."),
  cat(MullCXXCategory));

opt<std::string> tool::ReportName(
  "report-name",
  desc("Filename for the report (only for supported reporters). Defaults to <timestamp>.<extension>"),
  Optional,
  value_desc("filename"),
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
  value_desc("directory"),
  init("/tmp/mull-cache"),
  cat(MullCXXCategory));

opt<std::string> tool::CompilationDatabasePath(
  "compdb-path",
  desc("Path to a compilation database (compile_commands.json) for junk detection"),
  value_desc("filename"),
  Optional,
  cat(MullCXXCategory));

opt<std::string> tool::CompilationFlags(
  "compilation-flags",
  desc("Extra compilation flags for junk detection"),
  value_desc("string"),
  Optional,
  cat(MullCXXCategory));

opt<bool> tool::DryRunOption(
  "dry-run",
  desc("Skips real mutants execution. Disabled by default"),
  Optional,
  init(false),
  cat(MullCXXCategory));

list<MutatorsOptionIndex> tool::Mutators(
  "mutators",
  desc("Choose mutators:"),
  ZeroOrMore,
  value_desc("mutator"),
  cat(MullCXXCategory));

opt<TestFrameworkOptionIndex> tool::TestFrameworks(
  "test-framework",
  desc("Choose test framework:"),
  Required,
  value_desc("framework"),
  cat(MullCXXCategory));

list<std::string> tool::LDSearchPaths(
  "ld-search-path",
  desc("Library search path"),
  ZeroOrMore,
  value_desc("directory"),
  cat(MullCXXCategory));

list<std::string> tool::ExcludePaths(
  "exclude-path",
  desc("File/directory paths to ignore (supports regex)"),
  ZeroOrMore,
  value_desc("regex"),
  cat(MullCXXCategory));

list<std::string> tool::IncludePaths(
  "include-path",
  desc("File/directory paths to whitelist (supports regex)"),
  ZeroOrMore,
  value_desc("regex"),
  cat(MullCXXCategory));

opt<SandboxKind> tool::SandboxOption(
    "sandbox",
    desc("Choose sandbox approach:"),
    Optional,
    value_desc("sandbox"),
    init(SandboxKind::Timer),
    cat(MullCXXCategory));

list<ReporterKind> tool::ReportersOption(
  "reporters",
  desc("Choose reporters:"),
  ZeroOrMore,
  value_desc("reporter"),
  cat(MullCXXCategory));

opt<bool> tool::IDEReporterShowKilled(
  "ide-reporter-show-killed",
  desc("Makes IDEReporter to also report killed mutations (disabled by default)"),
  Optional,
  init(false),
  cat(MullCXXCategory));

opt<bool> tool::DumpCLIInterface(
    "dump-cli",
    desc("Prints CLI options in the Sphinx/RST friendly format"),
    Optional,
    init(false),
    cat(MullCXXCategory),
    Hidden);

opt<bool> tool::DumpMutators(
    "dump-mutators",
    desc("Prints available mutators in the Sphinx/RST friendly format"),
    Optional,
    init(false),
    cat(MullCXXCategory),
    Hidden);

// clang-format on

MutatorsCLIOptions::MutatorsCLIOptions(Diagnostics &diagnostics,
                                       list<MutatorsOptionIndex> &parameter)
    : factory(diagnostics), options(factory.commandLineOptions()), parameter(parameter) {
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

std::vector<std::pair<std::string, std::string>> &MutatorsCLIOptions::getOptions() {
  return options;
}

TestFrameworkCLIOptions::TestFrameworkCLIOptions(Diagnostics &diagnostics,
                                                 opt<TestFrameworkOptionIndex> &parameter)
    : diagnostics(diagnostics), options(factory.commandLineOptions()), parameter(parameter) {
  int index = 0;
  for (auto &option : options) {
    parameter.getParser().addLiteralOption(option.first.c_str(), index++, option.second.c_str());
  }
}

TestFramework TestFrameworkCLIOptions::testFramework(Toolchain &toolchain,
                                                     Configuration &configuration) {
  auto &name = options[parameter.getValue()].first;
  return factory.createTestFramework(name, toolchain, configuration, diagnostics);
}

struct SandboxDefinition {
  std::string name;
  std::string description;
  SandboxKind kind;
};

static std::vector<SandboxDefinition> sandboxes({
    { "None", "No sandboxing", SandboxKind::NoSandbox },
    { "Watchdog", "Uses 4 processes, not recommended", SandboxKind::Watchdog },
    { "Timer", "Fastest, Recommended", SandboxKind::Timer },
});

SandboxCLIOptions::SandboxCLIOptions(opt<SandboxKind> &parameter) : parameter(parameter) {
  for (auto &opt : sandboxes) {
    parameter.getParser().addLiteralOption(opt.name.c_str(), opt.kind, opt.description.c_str());
  }
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

struct ReporterDefinition {
  std::string name;
  std::string description;
  ReporterKind kind;
};

static std::vector<ReporterDefinition> reporterOptions({
    { "IDE", "Prints compiler-like warnings into stdout", ReporterKind::IDE },
    { "SQLite", "Saves results into an SQLite database", ReporterKind::SQLite },
    { "Elements",
      "Generates mutation-testing-elements compatible JSON file",
      ReporterKind::Elements },
});

ReportersCLIOptions::ReportersCLIOptions(Diagnostics &diagnostics, list<ReporterKind> &parameter)
    : diagnostics(diagnostics), parameter(parameter) {
  for (ReporterDefinition &opt : reporterOptions) {
    parameter.getParser().addLiteralOption(opt.name.c_str(), opt.kind, opt.description.c_str());
  }
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
      reporters.emplace_back(new mull::IDEReporter(diagnostics, IDEReporterShowKilled));
    } break;
    case ReporterKind::SQLite: {
      reporters.emplace_back(new mull::SQLiteReporter(diagnostics, directory, name));
    } break;
    case ReporterKind::Elements: {
      reporters.emplace_back(
          new mull::MutationTestingElementsReporter(diagnostics, directory, name, provider));
    } break;
    }
  }

  if (reporters.empty()) {
    reporters.emplace_back(new mull::IDEReporter(diagnostics, IDEReporterShowKilled));
  }

  return reporters;
}

static void sanitizeString(std::string &input, const std::string &replacement) {
  std::string escape("\\");
  auto pos = input.find(replacement);
  if (pos != std::string::npos) {
    input.replace(pos, replacement.size(), escape + replacement);
  }
}

static std::string sanitizeString(const std::string &input) {
  std::string result = input;
  sanitizeString(result, "|=");
  sanitizeString(result, "*=");
  return result;
}

void tool::dumpCLIInterface(Diagnostics &diagnostics) {
  // Enumerating CLI options explicitly to control the order and what to show
  Option *mutators = &(Option &)Mutators;
  Option *reporters = &(Option &)ReportersOption;
  std::vector<Option *> mullOptions({
      &Workers,
      &DryRunOption,

      &CacheDir,
      &DisableCache,

      &ReportName,
      &ReportDirectory,
      reporters,
      &IDEReporterShowKilled,

      &CompilationDatabasePath,
      &CompilationFlags,
      &(Option &)LDSearchPaths,
      &(Option &)IncludePaths,
      &(Option &)ExcludePaths,

      &SandboxOption,
      &TestFrameworks,

      mutators,
  });

  llvm::StringMap<Option *> opts = getRegisteredOptions();
  std::stringstream help;

  for (auto &option : mullOptions) {
    llvm::SmallVector<llvm::StringRef, 16> OptionNames;
    help << "--" << option->ArgStr.str();
    if (!option->ValueStr.empty()) {
      help << " " << option->ValueStr.str();
    }
    help << "\t\t" << option->HelpStr.str();
    help << "\n\n";

    if (option == mutators) {
      MutatorsFactory factory(diagnostics);
      factory.init();

      help << "    Groups:\n";
      for (const auto &pair : factory.getGroupsMapping()) {
        help << "      :" << pair.first << ":\t"
             << MutatorsFactory::descriptionForGroup(pair.second) << "\n\n";
      }

      help << "    Single mutators:\n";
      for (const auto &pair : factory.getMutatorsMapping()) {
        help << "      :" << pair.first << ":\t" << sanitizeString(pair.second->getDescription())
             << "\n\n";
      }
    }

    if (option == reporters) {
      for (ReporterDefinition &opt : reporterOptions) {
        help << "    :" << opt.name << ":\t" << opt.description << "\n\n";
      }
    }

    if (option == &SandboxOption) {
      for (auto &opt : sandboxes) {
        help << "    :" << opt.name << ":\t" << opt.description << "\n\n";
      }
    }

    if (option == &TestFrameworks) {
      TestFrameworkFactory factory;
      for (auto &pair : factory.commandLineOptions()) {
        help << "    :" << pair.first << ":\t" << pair.second << "\n\n";
      }
    }
  }
  llvm::outs() << help.str();
}

void tool::dumpMutators(Diagnostics &diagnostics) {
  MutatorsFactory factory(diagnostics);
  factory.init();

  std::vector<Mutator *> availableMutators;
  for (const auto &pair : factory.getMutatorsMapping()) {
    availableMutators.push_back(pair.second.get());
  }

  std::sort(availableMutators.begin(), availableMutators.end(), [&](Mutator *lhs, Mutator *rhs) {
    return lhs->getUniqueIdentifier() < rhs->getUniqueIdentifier();
  });

  std::stringstream replacements;
  std::stringstream table;

  std::string firstHeaderName("Operator Name");
  std::string firstHeader(firstHeaderName.size(), '=');
  std::string secondHeaderName("Operator Semantics");
  std::string secondHeader(secondHeaderName.size(), '=');

  table << firstHeader << " " << secondHeader << "\n";
  table << firstHeaderName << " " << secondHeaderName << "\n";
  table << firstHeader << " " << secondHeader << "\n";

  for (size_t i = 0; i < availableMutators.size(); i++) {
    Mutator *mutator = availableMutators[i];
    std::string n = std::to_string(i);
    std::string name("op" + n);
    std::string description("desc" + n);

    std::string padding(firstHeader.size() - name.size() - 1, ' ');
    table << "|" << name << "|" << padding << "|" << description << "|\n";

    replacements << ".. |" << name << "| replace:: " << mutator->getUniqueIdentifier() << "\n";
    replacements << ".. |" << description
                 << "| replace:: " << sanitizeString(mutator->getDescription()) << "\n";
  }

  replacements << "\n\n";
  table << firstHeader << " " << secondHeader << "\n";

  llvm::outs() << replacements.str();
  llvm::outs() << table.str();
}

#include "CLIOptions.h"
#include <mull/Diagnostics/Diagnostics.h>
#include <mull/Mutators/Mutator.h>
#include <mull/Reporters/IDEReporter.h>
#include <mull/Reporters/MutationTestingElementsReporter.h>
#include <mull/Reporters/SQLiteReporter.h>

using namespace mull;
using namespace tool;
using namespace llvm::cl;

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
      reporters.emplace_back(new mull::IDEReporter(diagnostics, params.IDEReporterShowKilled));
    } break;
    case ReporterKind::SQLite: {
      reporters.emplace_back(new mull::SQLiteReporter(diagnostics, directory, name));
    } break;
    case ReporterKind::Elements: {
      if (!params.compilationDatabaseAvailable) {
        diagnostics.warning("Mutation Testing Elements Reporter may not work without compilation "
                            "database. Consider providing -compdb-path or -compilation-flags.");
      }
      reporters.emplace_back(
          new mull::MutationTestingElementsReporter(diagnostics, directory, name, provider));
    } break;
    }
  }

  if (reporters.empty()) {
    reporters.emplace_back(new mull::IDEReporter(diagnostics, params.IDEReporterShowKilled));
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

void dumpCLIOption(std::stringstream &stream, llvm::cl::Option *option) {
  stream << "--" << option->ArgStr.str();
  if (!option->ValueStr.empty()) {
    stream << " " << option->ValueStr.str();
  }
  stream << "\t\t" << option->HelpStr.str();
  stream << "\n\n";
}

void dumpCLIReporters(std::stringstream &stream) {
  for (ReporterDefinition &opt : reporterOptions) {
    stream << "    :" << opt.name << ":\t" << opt.description << "\n\n";
  }
}

void dumpCLIMutators(std::stringstream &stream, mull::Diagnostics &diagnostics) {
  MutatorsFactory factory(diagnostics);
  factory.init();

  stream << "    Groups:\n";
  for (const auto &pair : factory.getGroupsMapping()) {
    stream << "      :" << pair.first << ":\t" << MutatorsFactory::descriptionForGroup(pair.second)
           << "\n\n";
  }

  stream << "    Single mutators:\n";
  for (const auto &pair : factory.getMutatorsMapping()) {
    stream << "      :" << pair.first << ":\t" << sanitizeString(pair.second->getDescription())
           << "\n\n";
  }
}

void tool::dumpCLIInterface(mull::Diagnostics &diagnostics,
                            const std::vector<llvm::cl::Option *> &options,
                            llvm::cl::Option *reporters, llvm::cl::Option *mutators) {
  std::stringstream help;
  for (llvm::cl::Option *option : options) {
    dumpCLIOption(help, option);

    if (option == mutators) {
      dumpCLIMutators(help, diagnostics);
    }

    if (option == reporters) {
      dumpCLIReporters(help);
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

#include "CLIOptions.h"
#include <mull/Mutators/Mutator.h>
#include <mull/Mutators/MutatorsFactory.h>
#include <mull/Reporters/GithubAnnotationsReporter.h>
#include <mull/Reporters/IDEReporter.h>
#include <mull/Reporters/MutationTestingElementsReporter.h>
#include <mull/Reporters/PatchesReporter.h>
#include <mull/Reporters/SQLiteReporter.h>

#include <llvm/Support/FileSystem.h>
#include <sstream>

using namespace mull;
using namespace tool;
using namespace std::string_literals;

std::vector<std::string> tool::toStdVector(const rust::Vec<rust::String> &v) {
  std::vector<std::string> out;
  out.reserve(v.size());
  for (const auto &s : v) {
    out.emplace_back(std::string(s));
  }
  return out;
}

ReporterParameters
tool::buildReporterParameters(const CliConfig &cli,
                              std::unordered_map<std::string, std::string> mullInformation) {
  return ReporterParameters{
    .reporters = toStdVector(cli.reporters),
    .reporterName = std::string(cli.report_name),
    .reporterDirectory = std::string(cli.report_dir),
    .patchBasePathDir = std::string(cli.report_patch_base),
    .compilationDatabaseAvailable = true,
    .IDEReporterShowKilled = cli.ide_reporter_show_killed,
    .mullInformation = std::move(mullInformation),
  };
}

std::vector<std::unique_ptr<Reporter>> tool::createReporters(const MullDiagnostics &diagnostics,
                                                             const ReporterParameters &params) {
  std::vector<std::unique_ptr<Reporter>> reporters;

  for (const auto &r : params.reporters) {
    if (r == "IDE") {
      reporters.emplace_back(new IDEReporter(diagnostics,
                                             params.IDEReporterShowKilled,
                                             params.reporterDirectory,
                                             params.reporterName));
    } else if (r == "SQLite") {
      reporters.emplace_back(new SQLiteReporter(
          diagnostics, params.reporterDirectory, params.reporterName, params.mullInformation));
    } else if (r == "Elements") {
      if (!params.compilationDatabaseAvailable) {
        diagnostics.warning("Mutation Testing Elements Reporter may not work without "
                            "compilation database. Consider providing -compdb-path or "
                            "-compilation-flags.");
      }
      reporters.emplace_back(new MutationTestingElementsReporter(
          diagnostics, params.reporterDirectory, params.reporterName, params.mullInformation));
    } else if (r == "Patches") {
      reporters.emplace_back(new PatchesReporter(diagnostics,
                                                 params.reporterDirectory,
                                                 params.reporterName,
                                                 params.patchBasePathDir,
                                                 params.mullInformation));
    } else if (r == "GithubAnnotations") {
      reporters.emplace_back(new GithubAnnotationsReporter(diagnostics));
    } else {
      diagnostics.warning("Unknown reporter: " + r);
    }
  }

  if (reporters.empty()) {
    reporters.emplace_back(new IDEReporter(
        diagnostics, params.IDEReporterShowKilled, params.reporterDirectory, params.reporterName));
  }

  return reporters;
}

rust::Vec<rust::String> tool::argsFromArgv(int argc, char **argv) {
  rust::Vec<rust::String> args;
  for (int i = 0; i < argc; i++) {
    args.push_back(rust::String(argv[i]));
  }
  return args;
}

void tool::dumpMutators(const MullDiagnostics &diagnostics, std::string out) {
  MutatorsFactory factory(diagnostics);
  factory.init();

  std::vector<Mutator *> availableMutators;
  for (const auto &pair : factory.getMutatorsMapping()) {
    availableMutators.push_back(pair.second.get());
  }

  std::sort(availableMutators.begin(), availableMutators.end(), [&](Mutator *lhs, Mutator *rhs) {
    return lhs->getUniqueIdentifier() < rhs->getUniqueIdentifier();
  });

  auto sanitizeString = [](const std::string &input) -> std::string {
    std::string result = input;
    auto replaceOne = [&](const std::string &replacement) {
      std::string escape("\\");
      auto pos = result.find(replacement);
      if (pos != std::string::npos) {
        result.replace(pos, replacement.size(), escape + replacement);
      }
    };
    replaceOne("|=");
    replaceOne("*=");
    return result;
  };

  std::stringstream replacements;
  std::stringstream table;

  std::string operatorHeaderName("Operator Name");
  std::string operatorHeader(operatorHeaderName.size(), '=');
  std::string semanticsHeaderName("Operator Semantics");
  std::string semanticsHeader(semanticsHeaderName.size(), '=');

  table << operatorHeader << " " << semanticsHeader << "\n";
  table << operatorHeaderName << " " << semanticsHeaderName << "\n";
  table << operatorHeader << " " << semanticsHeader << "\n";

  for (size_t i = 0; i < availableMutators.size(); i++) {
    Mutator *mutator = availableMutators[i];
    std::string n = std::to_string(i);
    std::string name("op" + n);
    std::string description("desc" + n);

    std::string padding(operatorHeader.size() - name.size() - 1, ' ');
    table << "|" << name << "|" << padding << "|" << description << "|\n";

    replacements << ".. |" << name << "| replace:: " << mutator->getUniqueIdentifier() << "\n";
    replacements << ".. |" << description
                 << "| replace:: " << sanitizeString(mutator->getDescription()) << "\n";
  }

  replacements << "\n\n";
  table << operatorHeader << " " << semanticsHeader << "\n";

  table << "\n\n";
  table << "Groups:\n";
  for (const auto &pair : factory.getGroupsMapping()) {
    table << "    :" << pair.first << ":\t" << MutatorsFactory::descriptionForGroup(pair.second)
          << "\n\n";
  }

  std::error_code ec;
  llvm::raw_fd_ostream os(out, ec);
  if (ec) {
    diagnostics.error(ec.message());
  }
  os << replacements.str();
  auto s = table.str();
  // trim trailing newline
  os << s.substr(0, s.size() - 1);
}
